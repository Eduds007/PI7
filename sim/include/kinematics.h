#ifndef PI7_SIM_KINEMATICS_H
#define PI7_SIM_KINEMATICS_H

typedef struct {
    double hip_rad;  /* junta do quadril no MuJoCo, absoluta, eixo (0 1 0) */
    double knee_rad; /* junta do joelho no MuJoCo, relativa a coxa, eixo (0 1 0) */
} ik_angles_t;

/*
 * Porte de src/pi7/trj_control/trj_control.c: tcl_generateSetpoint().
 * x_raw, y_raw sao os valores (x,y) brutos como armazenados na tabela de
 * trajetoria (ANTLR+MODBUS/GCode-example), ANTES do offset de -160 que o
 * firmware original aplica internamente (aplicado aqui do mesmo jeito).
 *
 * A convencao original do firmware (VAL = 90-gamma para o quadril,
 * VAL = 180-beta para o joelho) era o zero mecanico especifico dos
 * servos/PICs, sem sentido fora do hardware real. Aqui usamos
 * hip = gamma, knee = 180-beta diretamente como angulos de junta do
 * MuJoCo -- verificado numericamente (cinematica direta) contra os 27
 * pontos da trajetoria.
 */
ik_angles_t ik_compute(int x_raw, int y_raw);

#endif
