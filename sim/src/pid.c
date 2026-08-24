#include "pid.h"

/* pidx.X/pidx.X/pid.c: pid_scaleExcitation() */
#define KICKSTART 150.0
#define MAX_EXCITATION 1000.0
#define MIN_EXCITATION (-1000.0)

/*
 * Zona morta de erro, EXTRA em relacao ao pid.c original do PIC. O
 * PIC real dependia do atrito/folga mecanica do motor+redutor real para
 * nao ficar oscilando bang-bang perto do alvo (o chute de +-150 sozinho
 * causaria exatamente esse chacoalhar); o joint damping puramente viscoso
 * do MuJoCo nao reproduz esse atrito estatico. Dentro da zona morta a
 * excitacao vai a zero (sem o chute), permitindo assentar; fora dela o
 * comportamento e identico ao pid_pid() original.
 */
#define ERROR_DEADBAND_DEG 1.0

void pid_init(leg_pid_t *p, double kp, double ki, double kd, bool full_pid) {
    p->kp = kp;
    p->ki = ki;
    p->kd = kd;
    p->integral_error = 0.0;
    p->previous_error = 0.0;
    p->full_pid = full_pid;
    p->excitation = 0.0;
}

void pid_clear_error(leg_pid_t *p) {
    p->integral_error = 0.0;
    p->previous_error = 0.0;
}

static double scale_excitation(double activation) {
    double excitation = activation;
    if (excitation > 0.0) {
        excitation = KICKSTART + excitation;
    } else if (excitation < 0.0) {
        excitation = -KICKSTART + excitation;
    }
    if (excitation > MAX_EXCITATION) {
        excitation = MAX_EXCITATION;
    }
    if (excitation < MIN_EXCITATION) {
        excitation = MIN_EXCITATION;
    }
    return excitation;
}

double pid_tick(leg_pid_t *p, double setpoint_deg, double measured_deg, double dt_s) {
    double error = setpoint_deg - measured_deg;
    double activation = p->kp * error;

    if (p->full_pid) {
        p->integral_error += error * dt_s;
        double derivative = (dt_s > 0.0) ? (error - p->previous_error) / dt_s : 0.0;
        activation += p->ki * p->integral_error + p->kd * derivative;
    }
    p->previous_error = error;

    if (error > -ERROR_DEADBAND_DEG && error < ERROR_DEADBAND_DEG) {
        p->excitation = 0.0;
    } else {
        p->excitation = scale_excitation(activation);
    }
    return p->excitation;
}
