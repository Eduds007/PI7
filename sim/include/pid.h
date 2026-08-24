#ifndef PI7_SIM_PID_H
#define PI7_SIM_PID_H

#include <stdbool.h>

/*
 * Porte fiel de pidx.X/pidx.X/pid.c -- o controlador PID real que rodava em
 * cada PIC16F886 (um por junta). O firmware real so aplicava controle
 * proporcional: kIntegral/kDerivative existiam como setters (recebidos via
 * protocolo serial) mas nunca eram somados em pid_pid(). full_pid=false
 * (padrao) reproduz isso exatamente; full_pid=true aplica tambem os termos
 * I/D usando os ganhos que o gerador de trajetoria (serial_communication.py)
 * enviava e que o firmware real ignorava silenciosamente.
 */
typedef struct {
    double kp, ki, kd;
    double integral_error;
    double previous_error;
    bool full_pid;
    double excitation; /* ultima excitacao calculada, [-1000, 1000] (escala do pid.c real) */
} leg_pid_t;

void pid_init(leg_pid_t *p, double kp, double ki, double kd, bool full_pid);
void pid_clear_error(leg_pid_t *p);

/*
 * Um tick de PID_INTERVAL (100ms), espelhando pid_pid() de pidx.X/pid.c:
 *   error = setpoint_deg - measured_deg
 *   activation = kp*error  (+ ki*integral + kd*derivada, se full_pid)
 *   excitation = scale_excitation(activation)
 * onde scale_excitation reproduz pid_scaleExcitation(): soma um "chute"
 * de +-150 (de uma faixa de +-1000) quando a ativacao e nao-nula --
 * o motor real so girava a partir de ~15% de duty cycle -- e satura em
 * +-1000. dt_s e o tempo real entre ticks, usado so quando full_pid=true.
 */
double pid_tick(leg_pid_t *p, double setpoint_deg, double measured_deg, double dt_s);

#endif
