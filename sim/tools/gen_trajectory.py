#!/usr/bin/env python3
"""Gera um programa G-code (formato ANTLR+MODBUS/GCode-example) para a perna
PI7 passar por cima de um obstaculo, a partir da geometria da perna e do
obstaculo (em vez de pontos medidos a mao).

Convencao de coordenadas (identica a sim/src/kinematics.c:ik_compute):
  x_raw, y_raw  = coordenadas brutas do G-code (o que vai em cada linha N...)
  x = x_raw - 160                (posicao horizontal do pe, relativa ao quadril)
  y = y_raw                      (distancia do pe ABAIXO do quadril; y menor = pe mais alto)
  altura do pe acima do chao     = hip_height_mm - y_raw   (quadril fixo no espaco)

Uso tipico:
  python3 sim/tools/gen_trajectory.py > ANTLR+MODBUS/GCode-generated
  python3 sim/tools/gen_trajectory.py --num-points 40 --clearance 50 --out foo.gcode
"""
import argparse
import math
import sys


def smoothstep(t: float) -> float:
    t = min(1.0, max(0.0, t))
    return t * t * (3.0 - 2.0 * t)


class LegGeometry:
    def __init__(self, l1_mm: float, l2_mm: float, hip_height_mm: float,
                 hip_range_deg, knee_range_deg):
        self.l1 = l1_mm
        self.l2 = l2_mm
        self.hip_height = hip_height_mm
        self.hip_range = hip_range_deg
        self.knee_range = knee_range_deg

    def reach_range(self):
        return (abs(self.l1 - self.l2), self.l1 + self.l2)

    def check_point(self, x_raw: int, y_raw: int):
        """Espelha (em ponto flutuante, sem o truncamento inteiro do firmware)
        a IK de sim/src/kinematics.c:ik_compute(), para validar se (x_raw,y_raw)
        e fisicamente alcancavel dentro dos limites de junta do model/leg.xml.
        Devolve (hip_deg, knee_deg, lista_de_problemas)."""
        x = x_raw - 160
        y = y_raw
        l3 = math.hypot(x, y)
        problems = []

        lo, hi = self.reach_range()
        if not (lo <= l3 <= hi):
            problems.append(f"L3={l3:.1f}mm fora do alcance [{lo:.1f}, {hi:.1f}]mm")
            return None, None, problems

        cos_beta = (self.l1 ** 2 + self.l2 ** 2 - l3 ** 2) / (2 * self.l1 * self.l2)
        cos_beta = min(1.0, max(-1.0, cos_beta))
        beta_deg = math.degrees(math.acos(cos_beta))
        knee_deg = 180.0 - beta_deg

        cos_gamma = (self.l1 ** 2 + l3 ** 2 - self.l2 ** 2) / (2 * self.l1 * l3)
        cos_gamma = min(1.0, max(-1.0, cos_gamma))
        delta_deg = math.degrees(math.acos(cos_gamma))
        phi_deg = math.degrees(math.atan2(y, x))
        hip_deg = phi_deg - delta_deg

        if not (self.hip_range[0] <= hip_deg <= self.hip_range[1]):
            problems.append(f"hip={hip_deg:.1f} fora de [{self.hip_range[0]}, {self.hip_range[1]}]")
        if not (self.knee_range[0] <= knee_deg <= self.knee_range[1]):
            problems.append(f"knee={knee_deg:.1f} fora de [{self.knee_range[0]}, {self.knee_range[1]}]")

        return hip_deg, knee_deg, problems


class ClearanceProfile:
    """Perfil de altura do pe em funcao da posicao horizontal (x, ja relativo
    ao quadril, isto e x = x_raw - 160). Garante por construcao que a altura
    e >= topo_do_obstaculo + margem em toda a faixa horizontal ocupada pelo
    obstaculo (mais a folga do raio do pe), independente de como os pontos
    forem espacados no tempo."""

    def __init__(self, stance_height_mm, peak_height_mm,
                 rise_lo, clear_lo, clear_hi, fall_hi):
        self.stance_h = stance_height_mm
        self.peak_h = peak_height_mm
        self.rise_lo = rise_lo
        self.clear_lo = clear_lo
        self.clear_hi = clear_hi
        self.fall_hi = fall_hi

    def height_at(self, x: float) -> float:
        if x <= self.rise_lo:
            return self.stance_h
        if x < self.clear_lo:
            t = (x - self.rise_lo) / (self.clear_lo - self.rise_lo)
            return self.stance_h + (self.peak_h - self.stance_h) * smoothstep(t)
        if x <= self.clear_hi:
            return self.peak_h
        if x < self.fall_hi:
            t = (x - self.clear_hi) / (self.fall_hi - self.clear_hi)
            return self.peak_h + (self.stance_h - self.peak_h) * smoothstep(t)
        return self.stance_h


def build_profile(x_start, x_end, obst_x_min, obst_x_max, obst_height_mm,
                   foot_radius_mm, clearance_margin_mm, stance_height_mm,
                   ramp_span_mm):
    clear_lo = obst_x_min - foot_radius_mm
    clear_hi = obst_x_max + foot_radius_mm
    rise_lo = clear_lo - ramp_span_mm
    fall_hi = clear_hi + ramp_span_mm

    if not (x_start <= rise_lo and fall_hi <= x_end):
        raise SystemExit(
            "geometria incompativel: nao ha espaco horizontal suficiente entre "
            f"x_start={x_start:.1f} e x_end={x_end:.1f} para subir/descer em torno "
            f"do obstaculo (precisaria de [{rise_lo:.1f}, {fall_hi:.1f}]). "
            "Aumente --x-start/--x-end, ou diminua --ramp-span."
        )

    peak_height_mm = obst_height_mm + clearance_margin_mm
    return ClearanceProfile(stance_height_mm, peak_height_mm, rise_lo, clear_lo, clear_hi, fall_hi)


def generate_points(geo: LegGeometry, profile: ClearanceProfile, x_start, x_end, num_points):
    points = []
    for i in range(num_points):
        s = i / (num_points - 1)
        x_world = x_start + (x_end - x_start) * smoothstep(s)
        height = profile.height_at(x_world)
        y_world = geo.hip_height - height  # distancia abaixo do quadril

        x_raw = round(x_world + 160)
        y_raw = round(y_world)
        points.append((x_raw, y_raw))
    return points


def validate_points(geo: LegGeometry, points):
    errors = []
    for i, (x_raw, y_raw) in enumerate(points, start=1):
        if not (0 <= x_raw <= 999 and 0 <= y_raw <= 999):
            errors.append(f"N{i:03d}: X{x_raw} Y{y_raw} fora do formato de 3 digitos do G-code")
            continue
        hip_deg, knee_deg, problems = geo.check_point(x_raw, y_raw)
        if problems:
            errors.append(f"N{i:03d}: X{x_raw:03d} Y{y_raw:03d} -> " + "; ".join(problems))
    return errors


def format_gcode(points) -> str:
    lines = []
    for i, (x_raw, y_raw) in enumerate(points, start=1):
        lines.append(f"N{i:03d} G01 X{x_raw:03d} Y{y_raw:03d}")
    lines.append(f"N{len(points) + 1:03d} M30")
    return "\r\n".join(lines) + "\r\n"


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)

    geo = p.add_argument_group("geometria da perna (default = sim/src/kinematics.c + model/leg.xml)")
    geo.add_argument("--l1", type=float, default=210.0, help="coxa, quadril->joelho (mm)")
    geo.add_argument("--l2", type=float, default=247.0, help="canela, joelho->pe (mm)")
    geo.add_argument("--hip-height", type=float, default=450.0, help="altura do quadril acima do chao (mm)")
    geo.add_argument("--hip-range", type=float, nargs=2, default=(0.0, 120.0), metavar=("MIN", "MAX"))
    geo.add_argument("--knee-range", type=float, nargs=2, default=(0.0, 130.0), metavar=("MIN", "MAX"))

    obst = p.add_argument_group("geometria do obstaculo (default = model/leg.xml)")
    obst.add_argument("--obst-x-min", type=float, default=-45.0, help="borda do obstaculo mais proxima do quadril (mm)")
    obst.add_argument("--obst-x-max", type=float, default=59.0, help="borda do obstaculo mais distante do quadril (mm)")
    obst.add_argument("--obst-height", type=float, default=50.0, help="altura do obstaculo (mm)")

    path = p.add_argument_group("forma da trajetoria")
    path.add_argument("--x-start", type=float, default=-160.0, help="posicao horizontal inicial do pe, relativa ao quadril (mm)")
    path.add_argument("--x-end", type=float, default=160.0, help="posicao horizontal final do pe, relativa ao quadril (mm)")
    path.add_argument("--stance-height", type=float, default=30.0, help="altura do pe em pe/apoio, inicio e fim (mm)")
    path.add_argument("--clearance", type=float, default=30.0, help="folga vertical acima do topo do obstaculo no pico (mm)")
    path.add_argument("--foot-radius", type=float, default=15.0, help="raio efetivo do pe, folga horizontal extra (mm)")
    path.add_argument("--ramp-span", type=float, default=70.0, help="distancia horizontal para subir/descer ate o pico (mm)")
    path.add_argument("--num-points", type=int, default=27, help="quantidade de pontos N001..N0NN gerados")

    p.add_argument("--out", type=argparse.FileType("w"), default=sys.stdout,
                    help="arquivo de saida (default: stdout). Usa apenas o formato G-code puro, sem comentarios.")
    p.add_argument("--allow-invalid", action="store_true",
                    help="emite o G-code mesmo se algum ponto falhar a validacao de alcance/junta (nao recomendado)")

    args = p.parse_args()

    geometry = LegGeometry(args.l1, args.l2, args.hip_height, tuple(args.hip_range), tuple(args.knee_range))
    profile = build_profile(
        x_start=args.x_start, x_end=args.x_end,
        obst_x_min=args.obst_x_min, obst_x_max=args.obst_x_max, obst_height_mm=args.obst_height,
        foot_radius_mm=args.foot_radius, clearance_margin_mm=args.clearance,
        stance_height_mm=args.stance_height, ramp_span_mm=args.ramp_span,
    )
    points = generate_points(geometry, profile, args.x_start, args.x_end, args.num_points)

    errors = validate_points(geometry, points)
    if errors:
        print(f"{len(errors)} ponto(s) invalido(s):", file=sys.stderr)
        for e in errors:
            print("  " + e, file=sys.stderr)
        if not args.allow_invalid:
            raise SystemExit("abortado (use --allow-invalid para gerar mesmo assim, ou ajuste os parametros)")

    peak_h = args.obst_height + args.clearance
    print(f"gerado: {args.num_points} pontos, x_raw [{points[0][0]}..{points[-1][0]}], "
          f"pico de altura {peak_h:.1f}mm (obstaculo {args.obst_height:.1f}mm + folga {args.clearance:.1f}mm)",
          file=sys.stderr)

    args.out.write(format_gcode(points))


if __name__ == "__main__":
    main()
