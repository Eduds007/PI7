# Simulação MuJoCo da perna PI7

Reconstrução em C + MuJoCo da perna de bancada de 2 elos (coxa + canela) do
projeto PI7, reaproveitando o máximo possível do código e dos dados originais
do repositório em vez de reinventar do zero:

- **Cinemática inversa**: porte literal (mesma aritmética inteira com
  truncamento) de `src/pi7/trj_control/trj_control.c` (`tcl_generateSetpoint`).
- **Trajetória**: os 27 pontos reais de `ANTLR+MODBUS/GCode-example` (o passo
  por cima de um objeto), embutidos como array estático.
- **Controlador PID**: porte literal de `pidx.X/pidx.X/pid.c` — o firmware que
  de fato rodava em cada PIC16F886 (um por junta). Esse projeto MPLAB X não
  está versionado neste repositório git, mas existe em disco em
  `pidx.X/pidx.X/`. Ele revelou que o controlador **só usava proporcional**
  (havia setters para Ki/Kd, recebidos via protocolo serial, mas nunca eram
  somados em `pid_pid()`), com um truque de "chute inicial" de ±150 (de uma
  faixa ±1000) porque o motor real só girava a partir de ~15% de duty cycle,
  e rodava a 100ms por junta (`PID_INTERVAL`), separado do tick de 200ms que
  gera os setpoints (`taskNCProcessing` em `src/main.c`).
- **Ganhos**: os defaults de `ANTLR+MODBUS/serial_communication.py`
  (`set_ganho`): quadril Kp=25 Ki=3 Kd=0, joelho Kp=5 Ki=1 Kd=0.

O enunciado da disciplina (`docs/Tema_PI7_2024_v2.pdf`, não versionado no
git) e o diagrama `docs/Conexoes PI-7 2024.pdf` confirmam o desenho: é uma
maquete de
**prótese de membro inferior** (não um robô genérico), 2 graus de liberdade
(quadril+joelho) montados sobre uma base fixa em "T invertido", escala
aproximada 1:2,5 de uma perna humana, motores acionados a 12V via ponte H
L298. Isso valida a decisão de quadril fixo no espaço usada aqui. O
enunciado tambem cita as dimensões do obstáculo em escala humana
(altura 260mm, largura 60mm). O obstáculo em `model/leg.xml` (50x104mm,
centrado em x=7mm) foi posicionado a partir de dados reais da simulação —
não de suposição: rodei a trajetória completa e gravei a altura mínima do
pé em faixas de 5mm de x (`/tmp/dbg_safezone.c`-style scan), achando a
janela onde o pé fica de forma confiável acima de 100mm (x entre -45mm e
+60mm) — e o obstáculo foi colocado dentro dela.

## Build

```bash
# 1. SDK do MuJoCo (uma vez só; ~32MB, extraído em third_party/, gitignored)
mkdir -p sim/third_party && cd sim/third_party
curl -LO https://github.com/google-deepmind/mujoco/releases/download/3.12.0/mujoco-3.12.0-linux-x86_64.tar.gz
mkdir mujoco && tar -xzf mujoco-3.12.0-linux-x86_64.tar.gz -C mujoco --strip-components 1
rm mujoco-3.12.0-linux-x86_64.tar.gz
cd ../..

# 2. dependencias de sistema (GLFW + OpenGL dev)
sudo apt-get install -y libglfw3-dev libglfw3 mesa-common-dev

# 3. build
cmake -S sim -B sim/build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build sim/build -j"$(nproc)"
```

O tarball oficial do MuJoCo (linux-x86_64) não traz um `mujocoConfig.cmake`,
então `sim/CMakeLists.txt` monta um alvo `IMPORTED` manualmente a partir de
`include/` e `lib/` soltos dentro do SDK extraído.

## Rodar

```bash
cd sim
./build/test_kinematics             # checagem standalone da cinematica (sem MuJoCo)
./build/leg_view                    # viewer interativo (R reinicia o passo, ESC fecha)
./build/leg_record saida.mp4 30     # grava saida.mp4 a 30fps (headless, ffmpeg)
```

`leg_view`/`leg_record` esperam ser executados com `model/leg.xml` relativo
ao diretorio `sim/` (ou passe o caminho como argumento).

Para GIF a partir do mp4 gravado:
```bash
ffmpeg -i saida.mp4 -vf "fps=15,scale=480:-1:flags=lanczos" saida.gif
```

## O que é fiel ao original vs. o que foi assumido

**Fiel (portado diretamente do código/dados existentes):**
- Geometria: L1=210mm (coxa), L2=247mm (canela), offset de -160 em x.
- Mapeamento de ângulos verificado numericamente contra os 27 pontos da
  trajetória (ver `sim/tests/test_kinematics.c`).
- Lei de controle: `activation = kp * error` (proporcional puro), chute de
  ±150 antes de saturar em ±1000, exatamente como `pidx.X/pidx.X/pid.c`.
- Cadencia dupla: setpoint novo a cada 200ms (Pico), PID recalculado a cada
  100ms por junta (PIC) — sample-and-hold entre ticks, não a cada passo de
  física.
- Quirk de escala do encoder: o firmware real convertia pulsos para graus
  dividindo por 5 (aproximação de 1852 pulsos/rotação ÷ 360 ≈ 5,14,
  arredondado para baixo) — preservado em `measured_deg_from_rad()` em
  `sim_common.c`, o que causa um pequeno erro de regime permanente (~2,9%)
  identico ao do hardware real.
- Ganhos Kp/Ki/Kd de `set_ganho()`.

**Assumido (não existe no repositório nem no histórico git):**
- Massa/inércia dos elos (`model/leg.xml`): coxa 0,35kg, canela 0,20kg,
  estimativas para uma perna leve de bancada (aluminio/impressao 3D).
- Limite de torque dos atuadores: quadril ±3 N·m, joelho ±2 N·m — a
  excitação [-1000,1000] do PID real (que comandava duty cycle de PWM, não
  torque) é remapeada linearmente para essa faixa em `leg_sim_step()`, ja
  que nao ha dados suficientes do motor+redutor real para simular a
  eletromecanica do motor DC diretamente.
- Amortecimento das juntas (`damping="1.0"` no XML): o motor real tinha
  atrito/back-EMF inerentes que amorteciam o movimento; sem isso o sistema
  oscila violentamente sob controle puramente proporcional. Ajustado
  empiricamente (0.05 → 0.4 → 1.0) ate a perna seguir a trajetoria e
  assentar no setpoint final sem ciclo-limite.
- Zona morta de erro de 1° em `pid.c` (`ERROR_DEADBAND_DEG`), ausente no
  `pid.c` original do PIC: o chute de ±150 (de uma faixa ±1000) dispara em
  QUALQUER erro não-nulo, o que por si só sustenta um ciclo-limite perto do
  alvo — no hardware real, o atrito estático do motor/redutor absorvia
  erros pequenos sem gerar esse "chacoalhar"; o `damping` puramente viscoso
  do MuJoCo não reproduz esse atrito estático, daí a zona morta artificial.
- A simulacao comeca com as juntas ja na pose do primeiro setpoint (nao em
  qpos=0/coxa esticada na horizontal) — um sistema real ja estaria com o
  PID segurando essa pose antes do `CMD_START`, nao caindo de um repouso
  arbitrario horizontal que varria um arco largo e colidia com o obstaculo
  antes do controle "esquentar".
- Posicao/tamanho do obstaculo em `model/leg.xml` foram determinados a
  partir de uma reprodução puramente cinemática dos 27 pontos (sem
  física/PID) para achar o pico real do arco (~156mm perto de x=-28mm), e
  então confirmados contra a trajetória com PID rodando de verdade
  (checando contatos MuJoCo pé-obstáculo/pé-chão ao longo de toda a
  execução — zero contatos reais, só resíduos sub-milimétricos nas quinas,
  abaixo da resolução visual).
- `full_pid=false` por padrao nos dois binarios (fiel ao PIC real, que
  ignorava Ki/Kd); mude para `true` na chamada de `leg_sim_load()` em
  `main_viewer.c`/`main_record.c` para experimentar com o PID completo.

O programa de 27 pontos e um unico passo (nao um ciclo de marcha completo):
ele termina no ar, perto do ponto 27, sem retornar automaticamente ao chao —
isso e fiel ao firmware original (`tcl_generateSetpoint` para apos a ultima
linha, sem repetir).
