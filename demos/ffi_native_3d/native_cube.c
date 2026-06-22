#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void draw_face(float r, float g, float b,
                      float ax, float ay, float az,
                      float bx, float by, float bz,
                      float cx, float cy, float cz,
                      float dx, float dy, float dz) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex3f(ax, ay, az);
    glVertex3f(bx, by, bz);
    glVertex3f(cx, cy, cz);
    glVertex3f(dx, dy, dz);
    glEnd();
}

static void draw_cube(void) {
    draw_face(0.92f, 0.18f, 0.24f, -1, -1,  1,  1, -1,  1,  1,  1,  1, -1,  1,  1);
    draw_face(0.12f, 0.60f, 0.95f, -1, -1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1);
    draw_face(0.20f, 0.78f, 0.35f, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1, -1);
    draw_face(0.98f, 0.72f, 0.18f, -1, -1, -1,  1, -1, -1,  1, -1,  1, -1, -1,  1);
    draw_face(0.70f, 0.32f, 0.92f,  1, -1, -1,  1,  1, -1,  1,  1,  1,  1, -1,  1);
    draw_face(0.95f, 0.42f, 0.16f, -1, -1, -1, -1, -1,  1, -1,  1,  1, -1,  1, -1);
}

int main(int argc, char** argv) {
    double seconds = 0.0;
    if (argc > 1) {
        seconds = atof(argv[1]);
    }

    if (!glfwInit()) {
        fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vyb Native 3D", NULL, NULL);
    if (!window) {
        fprintf(stderr, "glfwCreateWindow failed\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);

    double start = glfwGetTime();
    while (!glfwWindowShouldClose(window) && (seconds <= 0.0 || glfwGetTime() - start < seconds)) {
        int width = 0;
        int height = 0;
        double elapsed = glfwGetTime() - start;

        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) {
            height = 1;
        }

        glViewport(0, 0, width, height);
        glClearColor(0.06f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)width / (float)height;
        glFrustum(-aspect, aspect, -1.0, 1.0, 1.5, 20.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef((float)(elapsed * 48.0), 0.4f, 1.0f, 0.2f);
        draw_cube();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
