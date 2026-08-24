#include "kinematics.h"
#include <math.h>

/* Constantes identicas a src/pi7/trj_control/trj_control.c */
#define IK_PI 31415 /* pi * 10000 */
#define IK_L1 210   /* mm, quadril->joelho ("coxa") */
#define IK_L2 247   /* mm, joelho->pe ("canela") */
#define IK_SCALE 100

ik_angles_t ik_compute(int x_raw, int y_raw) {
    int x = x_raw - 160;
    int y = y_raw;

    int L3 = (int)lround(sqrt((double)(x * x + y * y)));
    int L3s = IK_SCALE * L3;
    int L1s = IK_L1 * IK_SCALE;
    int L2s = IK_L2 * IK_SCALE;
    int L1_sq = L1s * L1s;
    int L2_sq = L2s * L2s;
    int L3_sq = L3s * L3s;

    int cos_beta_scaled = (L1_sq + L2_sq - L3_sq) / (2 * IK_L1 * IK_L2);
    int beta_scaled = (int)lround((180.0 * 10000.0 / IK_PI) *
                                   acos((double)cos_beta_scaled / (IK_SCALE * IK_SCALE)));

    int gamma = 0;
    int thetas = 0;
    if (x == 0) {
        gamma = 90;
    } else {
        thetas = (int)(IK_SCALE * atan((double)y / x));
        if (x < 0) {
            gamma = 180;
        }
    }

    int cos_gamma_scaled = (L1_sq + L3_sq - L2_sq) / (2 * IK_L1 * L3);
    gamma += (int)((180.0 * 10000.0 / IK_PI) *
                    ((double)thetas / IK_SCALE -
                     acos((double)cos_gamma_scaled / (IK_SCALE * IK_SCALE))));

    ik_angles_t out;
    out.hip_rad = gamma * (M_PI / 180.0);
    out.knee_rad = (180.0 - beta_scaled) * (M_PI / 180.0);
    return out;
}
