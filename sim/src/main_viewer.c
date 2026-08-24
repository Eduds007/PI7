#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <stdio.h>

#include "sim_common.h"

static leg_sim_t g_sim;
static mjvCamera cam;
static mjvOption opt;
static mjvScene scn;
static mjrContext con;

static bool button_left = false, button_middle = false, button_right = false;
static double lastx = 0, lasty = 0;

static void keyboard_cb(GLFWwindow *window, int key, int scancode, int act, int mods) {
    (void)scancode;
    (void)mods;
    if (act == GLFW_PRESS && key == GLFW_KEY_R) {
        traj_start(&g_sim.traj);
    }
    if (act == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, 1);
    }
}

static void mouse_button_cb(GLFWwindow *window, int button, int act, int mods) {
    (void)button;
    (void)act;
    (void)mods;
    button_left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    button_middle = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    button_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    glfwGetCursorPos(window, &lastx, &lasty);
}

static void mouse_move_cb(GLFWwindow *window, double xpos, double ypos) {
    if (!button_left && !button_middle && !button_right) {
        return;
    }
    double dx = xpos - lastx;
    double dy = ypos - lasty;
    lastx = xpos;
    lasty = ypos;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    bool mod_shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    mjtMouse action;
    if (button_right) {
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    } else if (button_left) {
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    } else {
        action = mjMOUSE_ZOOM;
    }

    mjv_moveCamera(g_sim.m, action, dx / height, dy / height, &cam);
}

static void scroll_cb(GLFWwindow *window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    mjv_moveCamera(g_sim.m, mjMOUSE_ZOOM, 0, -0.05 * yoffset, &cam);
}

int main(int argc, char **argv) {
    const char *xml_path = (argc > 1) ? argv[1] : "model/leg.xml";

    leg_sim_load(&g_sim, xml_path, /*interpolate_traj=*/true, /*full_pid=*/false);

    if (!glfwInit()) {
        fprintf(stderr, "glfwInit falhou\n");
        return 1;
    }
    GLFWwindow *window = glfwCreateWindow(1280, 720, "PI7 Leg - MuJoCo (R = reiniciar passo)", NULL, NULL);
    if (!window) {
        fprintf(stderr, "glfwCreateWindow falhou\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);
    mjv_makeScene(g_sim.m, &scn, 2000);
    mjr_makeContext(g_sim.m, &con, mjFONTSCALE_150);

    cam.distance = 1.2;
    cam.azimuth = 90;
    cam.elevation = -15;
    cam.lookat[0] = 0;
    cam.lookat[1] = 0;
    cam.lookat[2] = 0.3;

    glfwSetKeyCallback(window, keyboard_cb);
    glfwSetMouseButtonCallback(window, mouse_button_cb);
    glfwSetCursorPosCallback(window, mouse_move_cb);
    glfwSetScrollCallback(window, scroll_cb);

    while (!glfwWindowShouldClose(window)) {
        double sim_start = g_sim.d->time;
        while (g_sim.d->time - sim_start < 1.0 / 60.0) {
            leg_sim_step(&g_sim);
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        mjrRect viewport = {0, 0, width, height};

        mjv_updateScene(g_sim.m, g_sim.d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    mjv_freeScene(&scn);
    mjr_freeContext(&con);
    leg_sim_free(&g_sim);
    glfwTerminate();
    return 0;
}
