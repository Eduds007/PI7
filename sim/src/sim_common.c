#include "sim_common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kinematics.h"

#define PID_TICK_S 0.100 /* pidx.X/pidx.X/tasks.c: PID_INTERVAL = 100 (ms) */
#define ENCODER_PULSES_PER_REV 1852.0 /* pidx.X/pidx.X/README.md */
#define ENCODER_DIVISOR 5.0 /* pid.c real: currentPosition/5 (~1852/360=5.14 arredondado) */

/*
 * Espelha pid_pid(): error = setPoint - currentPosition/5, onde
 * currentPosition sao pulsos brutos do encoder. Aqui pulamos a contagem
 * discreta de pulsos (o MuJoCo ja da o angulo continuo exato) mas
 * mantemos a mesma conversao aproximada /5 (em vez de /5.144 exato),
 * reproduzindo o mesmo erro de escala de ~2,9% que o firmware real tinha.
 */
static double measured_deg_from_rad(double joint_rad) {
    double joint_deg = joint_rad * (180.0 / M_PI);
    return joint_deg * (ENCODER_PULSES_PER_REV / 360.0) / ENCODER_DIVISOR;
}

void leg_sim_load(leg_sim_t *sim, const char *xml_path, bool interpolate_traj, bool full_pid) {
    char error[1024] = "";
    sim->m = mj_loadXML(xml_path, NULL, error, sizeof(error));
    if (!sim->m) {
        fprintf(stderr, "Falha ao carregar %s: %s\n", xml_path, error);
        exit(1);
    }
    sim->d = mj_makeData(sim->m);

    sim->hip_joint_id = mj_name2id(sim->m, mjOBJ_JOINT, "hip");
    sim->knee_joint_id = mj_name2id(sim->m, mjOBJ_JOINT, "knee");
    sim->hip_actuator_id = mj_name2id(sim->m, mjOBJ_ACTUATOR, "hip_motor");
    sim->knee_actuator_id = mj_name2id(sim->m, mjOBJ_ACTUATOR, "knee_motor");
    if (sim->hip_joint_id < 0 || sim->knee_joint_id < 0 || sim->hip_actuator_id < 0 ||
        sim->knee_actuator_id < 0) {
        fprintf(stderr,
                "%s nao tem os joints/actuators esperados 'hip'/'knee'/'hip_motor'/'knee_motor'\n",
                xml_path);
        exit(1);
    }

    traj_init(&sim->traj, interpolate_traj);
    traj_start(&sim->traj);

    /* ANTLR+MODBUS/serial_communication.py: set_ganho(kpa=25,kia=3,kda=0,kpb=5,kib=1,kdb=0) */
    pid_init(&sim->pid_hip, 25.0, 3.0, 0.0, full_pid);
    pid_init(&sim->pid_knee, 5.0, 1.0, 0.0, full_pid);

    sim->max_torque_hip = 3.0;  /* Nm assumido, ver sim/README.md */
    sim->max_torque_knee = 2.0; /* Nm assumido, ver sim/README.md */
    sim->pid_accum_s = 0.0;

    int x0, y0;
    traj_update(&sim->traj, 0.0, &x0, &y0);
    ik_angles_t a0 = ik_compute(x0, y0);
    sim->hip_setpoint_deg = a0.hip_rad * 180.0 / M_PI;
    sim->knee_setpoint_deg = a0.knee_rad * 180.0 / M_PI;

    /* Comeca ja na pose do primeiro setpoint em vez de qpos=0 (coxa na
     * horizontal): um sistema real ja estaria com o PID segurando essa
     * pose antes do CMD_START, nao caindo de um repouso arbitrario. Sem
     * isso, a queda inicial descontrolada varre um arco largo que pode
     * colidir com o obstaculo antes do controle "esquentar". */
    sim->d->qpos[sim->m->jnt_qposadr[sim->hip_joint_id]] = a0.hip_rad;
    sim->d->qpos[sim->m->jnt_qposadr[sim->knee_joint_id]] = a0.knee_rad;
    mj_forward(sim->m, sim->d);
}

void leg_sim_step(leg_sim_t *sim) {
    double dt = sim->m->opt.timestep;

    /* tick de trajetoria de 200ms -> novo setpoint via IK (espelha taskNCProcessing) */
    int x, y;
    traj_update(&sim->traj, dt, &x, &y);
    ik_angles_t a = ik_compute(x, y);
    sim->hip_setpoint_deg = a.hip_rad * 180.0 / M_PI;
    sim->knee_setpoint_deg = a.knee_rad * 180.0 / M_PI;

    /* tick de PID de 100ms por junta (espelha PID_INTERVAL em cada PIC) */
    sim->pid_accum_s += dt;
    if (sim->pid_accum_s + 1e-9 >= PID_TICK_S) {
        sim->pid_accum_s -= PID_TICK_S;

        double hip_meas =
            measured_deg_from_rad(sim->d->qpos[sim->m->jnt_qposadr[sim->hip_joint_id]]);
        double knee_meas =
            measured_deg_from_rad(sim->d->qpos[sim->m->jnt_qposadr[sim->knee_joint_id]]);

        double hip_exc = pid_tick(&sim->pid_hip, sim->hip_setpoint_deg, hip_meas, PID_TICK_S);
        double knee_exc = pid_tick(&sim->pid_knee, sim->knee_setpoint_deg, knee_meas, PID_TICK_S);

        sim->d->ctrl[sim->hip_actuator_id] = (hip_exc / 1000.0) * sim->max_torque_hip;
        sim->d->ctrl[sim->knee_actuator_id] = (knee_exc / 1000.0) * sim->max_torque_knee;
    }

    mj_step(sim->m, sim->d);
}

bool leg_sim_program_finished(const leg_sim_t *sim) {
    return traj_finished(&sim->traj);
}

void leg_sim_free(leg_sim_t *sim) {
    mj_deleteData(sim->d);
    mj_deleteModel(sim->m);
}
