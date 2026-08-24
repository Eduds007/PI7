#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <stdio.h>
#include <stdlib.h>

#include "sim_common.h"

#define WIDTH 1280
#define HEIGHT 720

int main(int argc, char **argv) {
    const char *out_path = (argc > 1) ? argv[1] : "pi7_leg.mp4";
    int fps = (argc > 2) ? atoi(argv[2]) : 30;
    const char *xml_path = (argc > 3) ? argv[3] : "model/leg.xml";

    leg_sim_t sim;
    leg_sim_load(&sim, xml_path, /*interpolate_traj=*/true, /*full_pid=*/false);

    /* MuJoCo precisa de um contexto OpenGL ativo mesmo para renderizar
     * offscreen; criamos uma janela GLFW oculta so para isso. */
    if (!glfwInit()) {
        fprintf(stderr, "glfwInit falhou\n");
        return 1;
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "offscreen", NULL, NULL);
    if (!window) {
        fprintf(stderr, "glfwCreateWindow (oculta) falhou\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    mjvCamera cam;
    mjv_defaultCamera(&cam);
    cam.distance = 1.2;
    cam.azimuth = 90;
    cam.elevation = -15;
    cam.lookat[0] = 0;
    cam.lookat[1] = 0;
    cam.lookat[2] = 0.3;

    mjvOption opt;
    mjv_defaultOption(&opt);
    mjvScene scn;
    mjv_defaultScene(&scn);
    mjv_makeScene(sim.m, &scn, 2000);

    mjrContext con;
    mjr_defaultContext(&con);
    mjr_makeContext(sim.m, &con, mjFONTSCALE_150);
    mjr_setBuffer(mjFB_OFFSCREEN, &con);

    mjrRect viewport = {0, 0, WIDTH, HEIGHT};
    unsigned char *rgb = malloc((size_t)3 * WIDTH * HEIGHT);
    if (!rgb) {
        fprintf(stderr, "malloc do buffer de frame falhou\n");
        return 1;
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -loglevel error -f rawvideo -pixel_format rgb24 -video_size %dx%d "
             "-framerate %d -i - -vf vflip -pix_fmt yuv420p -c:v libx264 -crf 18 \"%s\"",
             WIDTH, HEIGHT, fps, out_path);
    FILE *ff = popen(cmd, "w");
    if (!ff) {
        fprintf(stderr, "nao foi possivel iniciar o ffmpeg (esta instalado?)\n");
        return 1;
    }

    /* 27 pontos x 200ms = 5.4s de trajetoria + folga para o PID assentar no ultimo setpoint */
    double record_seconds = TRAJ_NUM_POINTS * TRAJ_TICK_S + 1.5;
    double next_frame_time = 0.0;
    double frame_dt = 1.0 / fps;

    while (sim.d->time < record_seconds) {
        leg_sim_step(&sim);
        if (sim.d->time >= next_frame_time) {
            mjv_updateScene(sim.m, sim.d, &opt, NULL, &cam, mjCAT_ALL, &scn);
            mjr_render(viewport, &scn, &con);
            mjr_readPixels(rgb, NULL, viewport, &con);
            fwrite(rgb, 1, (size_t)3 * WIDTH * HEIGHT, ff);
            next_frame_time += frame_dt;
        }
    }

    pclose(ff);
    free(rgb);
    mjv_freeScene(&scn);
    mjr_freeContext(&con);
    leg_sim_free(&sim);
    glfwDestroyWindow(window);
    glfwTerminate();

    printf("Gravado em %s (%.1fs, %dfps)\n", out_path, record_seconds, fps);
    return 0;
}
