#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "kinematics.h"

static int g_failures = 0;

static void expect_near(const char *what, double got, double expected, double tol) {
    if (fabs(got - expected) >= tol) {
        fprintf(stderr, "FALHA: %s = %.3f, esperado %.3f (tol %.3f)\n", what, got, expected, tol);
        g_failures++;
    }
}

static void expect_range(const char *what, double got, double lo, double hi) {
    if (got < lo || got > hi) {
        fprintf(stderr, "FALHA: %s = %.3f fora do intervalo [%.3f, %.3f]\n", what, got, lo, hi);
        g_failures++;
    }
}

/*
 * Valores de referencia obtidos a partir da propria aritmetica inteira
 * (com truncamento) de src/pi7/trj_control/trj_control.c, conferidos a
 * mao passo a passo para o ponto "home" (ver historico de implementacao).
 * O truncamento em cada etapa intermediaria e uma caracteristica fiel do
 * firmware original (tudo em int), entao os graus resultantes sao
 * numeros inteiros exatos, nao os valores em ponto flutuante "ideais".
 */
static void check(const char *label, int x, int y, double exp_hip_deg, double exp_knee_deg) {
    ik_angles_t a = ik_compute(x, y);
    double hip_deg = a.hip_rad * 180.0 / M_PI;
    double knee_deg = a.knee_rad * 180.0 / M_PI;
    printf("%-12s raw(%4d,%4d) -> hip=%6.2f (esperado %6.2f)  knee=%6.2f (esperado %6.2f)\n", label,
           x, y, hip_deg, exp_hip_deg, knee_deg, exp_knee_deg);
    expect_near(label, hip_deg, exp_hip_deg, 0.1);
    expect_near(label, knee_deg, exp_knee_deg, 0.1);
}

int main(void) {
    check("home", 0, 420, 100.0, 22.0);
    check("mid-swing", 64, 303, 57.0, 92.0);
    check("peak-lift", 102, 294, 47.0, 98.0);
    check("far-point", 314, 408, 49.0, 35.0);

    /* pose "home" deve ser uma perna de pe levemente flexionada, nao degenerada */
    ik_angles_t home = ik_compute(0, 420);
    double home_hip_deg = home.hip_rad * 180.0 / M_PI;
    double home_knee_deg = home.knee_rad * 180.0 / M_PI;
    expect_range("home_hip_deg", home_hip_deg, 80.0, 110.0);
    expect_range("home_knee_deg", home_knee_deg, 5.0, 45.0);

    if (g_failures > 0) {
        fprintf(stderr, "%d checagem(ns) falharam.\n", g_failures);
        return 1;
    }
    printf("OK: todos os pontos de checagem da cinematica batem.\n");
    return 0;
}
