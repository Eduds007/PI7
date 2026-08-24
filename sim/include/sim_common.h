#ifndef PI7_SIM_COMMON_H
#define PI7_SIM_COMMON_H

#include <mujoco/mujoco.h>
#include <stdbool.h>

#include "pid.h"
#include "trajectory.h"

typedef struct {
    mjModel *m;
    mjData *d;

    int hip_joint_id, knee_joint_id;
    int hip_actuator_id, knee_actuator_id;

    traj_state_t traj;
    leg_pid_t pid_hip, pid_knee;

    double hip_setpoint_deg, knee_setpoint_deg;
    double pid_accum_s; /* tempo desde o ultimo tick de PID de 100ms (PID_INTERVAL) */
    double max_torque_hip, max_torque_knee; /* Nm quando excitation = +-1000, ver sim/README.md */
} leg_sim_t;

/*
 * Carrega model/leg.xml, resolve os ids de junta/atuador, inicializa os
 * dois PIDs com os ganhos de ANTLR+MODBUS/serial_communication.py
 * (set_ganho: quadril Kp=25 Ki=3 Kd=0, joelho Kp=5 Ki=1 Kd=0) e inicia o
 * programa de trajetoria. full_pid seleciona se Ki/Kd sao de fato
 * aplicados (o PIC real nunca aplicava -- ver pid.h). Encerra o processo
 * com mensagem de erro se o modelo nao carregar.
 */
void leg_sim_load(leg_sim_t *sim, const char *xml_path, bool interpolate_traj, bool full_pid);

/*
 * Avanca a simulacao em exatamente um mj_step, tratando internamente o
 * tick de trajetoria de 200ms e o tick de PID de 100ms por junta
 * (sample-and-hold entre ticks, igual a cadencia real Pico (200ms) / PIC
 * (100ms)).
 */
void leg_sim_step(leg_sim_t *sim);

bool leg_sim_program_finished(const leg_sim_t *sim);

void leg_sim_free(leg_sim_t *sim);

#endif
