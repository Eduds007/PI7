#include "trajectory.h"

/*
 * ANTLR+MODBUS/GCode-example: N001..N027 (N028 e so "M30", fim de
 * programa, nao e um ponto). Coordenadas brutas (antes do offset -160
 * aplicado dentro de ik_compute()).
 */
static const traj_point_t g_trajectory[TRAJ_NUM_POINTS] = {
    {0, 420},   {6, 398},   {13, 378},  {19, 362},  {26, 348},  {32, 336},  {38, 327},
    {45, 319},  {51, 312},  {58, 307},  {64, 303},  {77, 298},  {102, 294}, {134, 295},
    {160, 297}, {186, 300}, {205, 304}, {218, 308}, {224, 310}, {230, 313}, {237, 316},
    {256, 330}, {269, 342}, {282, 357}, {294, 375}, {307, 396}, {314, 408},
};

void traj_init(traj_state_t *st, bool interpolate) {
    st->index = 0;
    st->running = false;
    st->interpolate = interpolate;
    st->t_in_program = 0.0;
}

void traj_start(traj_state_t *st) {
    st->index = 0;
    st->running = true;
    st->t_in_program = 0.0;
}

bool traj_finished(const traj_state_t *st) {
    return !st->running;
}

static void lerp_point(const traj_point_t *a, const traj_point_t *b, double frac, int *out_x,
                        int *out_y) {
    *out_x = (int)((double)a->x + frac * (double)(b->x - a->x));
    *out_y = (int)((double)a->y + frac * (double)(b->y - a->y));
}

void traj_update(traj_state_t *st, double dt_s, int *out_x, int *out_y) {
    if (!st->running) {
        const traj_point_t *last = &g_trajectory[TRAJ_NUM_POINTS - 1];
        *out_x = last->x;
        *out_y = last->y;
        return;
    }

    st->t_in_program += dt_s;
    int elapsed_ticks = (int)(st->t_in_program / TRAJ_TICK_S);

    if (elapsed_ticks >= TRAJ_NUM_POINTS) {
        st->index = TRAJ_NUM_POINTS - 1;
        st->running = false;
        *out_x = g_trajectory[st->index].x;
        *out_y = g_trajectory[st->index].y;
        return;
    }
    st->index = elapsed_ticks;

    if (st->interpolate && st->index + 1 < TRAJ_NUM_POINTS) {
        double frac = (st->t_in_program - st->index * TRAJ_TICK_S) / TRAJ_TICK_S;
        lerp_point(&g_trajectory[st->index], &g_trajectory[st->index + 1], frac, out_x, out_y);
    } else {
        *out_x = g_trajectory[st->index].x;
        *out_y = g_trajectory[st->index].y;
    }
}
