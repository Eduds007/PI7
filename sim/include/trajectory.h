#ifndef PI7_SIM_TRAJECTORY_H
#define PI7_SIM_TRAJECTORY_H

#include <stdbool.h>

#define TRAJ_NUM_POINTS 27
#define TRAJ_TICK_S 0.200 /* espelha o tick de 200ms de taskNCProcessing (src/main.c) */

typedef struct {
    int x;
    int y;
} traj_point_t;

typedef struct {
    int index;
    bool running;
    bool interpolate;
    double t_in_program; /* segundos desde traj_start() */
} traj_state_t;

void traj_init(traj_state_t *st, bool interpolate);
void traj_start(traj_state_t *st);
bool traj_finished(const traj_state_t *st);

/*
 * Avanca dt_s segundos de tempo de programa e devolve o alvo (x,y) atual
 * (coordenadas brutas do g-code, prontas para ik_compute()).
 *
 * interpolate=false: identico a tcl_generateSetpoint() -- avanca para o
 * proximo dos 27 pontos a cada 200ms, sem interpolacao (fiel ao firmware
 * original).
 * interpolate=true (padrao dos binarios leg_view/leg_record): interpola
 * linearmente entre o waypoint atual e o proximo dentro da janela de
 * 200ms -- um salto puro de setpoint a cada 200ms alimentando um PID
 * rapido de fisica fica visualmente seco/com solavancos; a interpolacao
 * evita isso ao custo de nao ser mais o comportamento literal do firmware.
 */
void traj_update(traj_state_t *st, double dt_s, int *out_x, int *out_y);

#endif
