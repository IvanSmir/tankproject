// main.cpp - Tank Transformer
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <chrono>
#include <string>

#include "include/Math3D.h"
#include "include/ShaderManager.h"
#include "Transformer.h" // Modificado
#include "include/Environment.h"
#include "include/HUD.h"

const char* VS_SRC = R"(
#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 model, view, projection;
out vec3 fragPos;
void main(){
    vec4 wp = model * vec4(aPos, 1.0);
    fragPos  = wp.xyz;
    gl_Position = projection * view * wp;
}
)";

const char* FS_SRC = R"(
#version 330 core
in vec3 fragPos;
uniform vec3 objectColor, lightPos, lightColor;
uniform bool lightOn;
out vec4 FragColor;
void main(){
    vec3 dx = dFdx(fragPos), dy = dFdy(fragPos);
    vec3 n = normalize(cross(dx, dy));
    float amb = 0.55;
    vec3 sun = normalize(vec3(1.0, 2.5, 1.0));
    float diff = max(dot(n, sun), 0.0) * 0.70;
    float extra = 0.0;
    if(lightOn){
        vec3 ld = normalize(lightPos - fragPos);
        extra = max(dot(n, ld), 0.0) * 0.6;
    }
    FragColor = vec4((amb + diff + extra) * lightColor * objectColor, 1.0);
}
)";

GLuint       g_shader = 0;
Transformer* g_tank = nullptr; // Modificado
Environment* g_env = nullptr;
HUD* g_hud = nullptr;
int          g_winW = 900;
int          g_winH = 650;
std::string  g_selPart;

static const Vec3 BODY_COLORS[] = {
    {0.22f, 0.35f, 0.13f},
    {0.55f, 0.20f, 0.12f},
    {0.15f, 0.25f, 0.55f},
    {0.65f, 0.55f, 0.10f},
    {0.20f, 0.20f, 0.20f}
};
static int g_colorIdx = 0;

static const std::string PARTS[] = {
    "turret", "cannon", "hatch", "trackL", "trackR",
    "body", "torso", "head", "armL", "armR", "legL", "legR", ""
};
static const int NUM_PARTS = 12;
static int g_partIdx = 0;

auto g_last = std::chrono::steady_clock::now();

void display() {
    glClearColor(0.08f, 0.10f, 0.14f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(g_shader);

    Mat4 proj = mat4Perspective(45.f, (float)g_winW / g_winH, 0.1f, 100.f);
    Mat4 view = g_env->viewMatrix();

    glUniformMatrix4fv(glGetUniformLocation(g_shader, "projection"), 1, GL_FALSE, proj.ptr());
    glUniformMatrix4fv(glGetUniformLocation(g_shader, "view"), 1, GL_FALSE, view.ptr());
    glUniform3f(glGetUniformLocation(g_shader, "lightPos"), 5.f, 10.f, 5.f);
    glUniform3f(glGetUniformLocation(g_shader, "lightColor"), 1.f, 1.f, 1.f);
    glUniform1i(glGetUniformLocation(g_shader, "lightOn"), g_tank->lightOn ? 1 : 0);

    g_env->drawFloor(g_shader);
    g_tank->draw(g_shader);

    // Modificado: Evaluar si es tanque basándonos en el modo actual o el objetivo
    bool isTankMode = (g_tank->currentMode == TransformMode::CAR) || (g_tank->targetMode == TransformMode::CAR);

    g_hud->draw(g_tank->walking, g_tank->talking, g_tank->lightOn,
        isTankMode, g_selPart, g_tank->shootCooldown);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    g_winW = w; g_winH = h;
    g_hud->windowW = w; g_hud->windowH = h;
    glViewport(0, 0, w, h);
}

void idle() {
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - g_last).count();
    g_last = now;
    if (dt > 0.05f) dt = 0.05f;
    g_tank->update(dt);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
    switch (tolower(key)) {
    case 'w': g_tank->startWalking(); break;
    case 's': g_tank->stopWalking();  break;
    case 'a': g_tank->rotateTurret(-5); g_selPart = "torso"; g_tank->selectPart("torso"); break;
    case 'd': g_tank->rotateTurret(5);  g_selPart = "torso"; g_tank->selectPart("torso"); break;
    case 'q': g_tank->elevateCannon(-3); g_selPart = "armR"; g_tank->selectPart("armR"); break;
    case 'e': g_tank->elevateCannon(3);  g_selPart = "armR"; g_tank->selectPart("armR"); break;
    case 'h': g_tank->toggleHatch(); g_selPart = "head"; g_tank->selectPart("head"); break;
    case 'f': g_tank->shoot(); break;
    case 'g': g_tank->greet(); break;
    case 'c':
        g_colorIdx = (g_colorIdx + 1) % 5;
        g_tank->setBodyColor(BODY_COLORS[g_colorIdx]);
        break;
    case 't':
        if (g_tank->currentMode == TransformMode::HUMANOID)
            g_tank->startTransformTo(TransformMode::CAR);
        else if (g_tank->currentMode == TransformMode::CAR)
            g_tank->startTransformTo(TransformMode::PLANE);
        else if (g_tank->currentMode == TransformMode::PLANE)
            g_tank->startTransformTo(TransformMode::BOAT);
        else
            g_tank->startTransformTo(TransformMode::HUMANOID);
        break;
    case 'x':
        g_tank->clearSelection(&g_tank->root);
        g_partIdx = (g_partIdx + 1) % (NUM_PARTS + 1);
        g_selPart = (g_partIdx < NUM_PARTS) ? PARTS[g_partIdx] : "";
        if (!g_selPart.empty()) g_tank->selectPart(g_selPart);
        break;
    case 'i': g_env->camPitch = g_env->camPitch > -80 ? g_env->camPitch - 2 : -80; break;
    case 'k': g_env->camPitch = g_env->camPitch < -5 ? g_env->camPitch + 2 : -5; break;
    case 'j': g_env->camYaw -= 3; break;
    case 'l': g_env->camYaw += 3; break;
    case 27: glutLeaveMainLoop(); break;
    }
}

void specialKeys(int key, int, int) {
    const float step = 0.5f;
    switch (key) {
    case GLUT_KEY_UP:    g_env->moveTarget(0, step); break;
    case GLUT_KEY_DOWN:  g_env->moveTarget(0, -step); break;
    case GLUT_KEY_LEFT:  g_env->moveTarget(-step, 0); break;
    case GLUT_KEY_RIGHT: g_env->moveTarget(step, 0); break;
    }
}

void mouse(int btn, int state, int x, int y) {
    g_env->onMouseButton(btn, state, x, y);
    if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        static int ti = 0;
        static const Vec3 TC[] = {
            {0.10f, 0.10f, 0.10f},
            {0.35f, 0.28f, 0.18f},
            {0.50f, 0.15f, 0.10f}
        };
        g_tank->setTrackColor(TC[ti++ % 3]);
    }
}

void mouseMotion(int x, int y) { g_env->onMouseMotion(x, y); }

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(g_winW, g_winH);
    glutCreateWindow("Tank Transformer - OpenGL 3.3");

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error GLEW: " << glewGetErrorString(err) << "\n";
        return -1;
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    glEnable(GL_DEPTH_TEST);
    g_shader = ShaderManager::loadFromSrc(VS_SRC, FS_SRC);
    if (!g_shader) { std::cerr << "Error al compilar shaders\n"; return -1; }

    // Modificado: Instanciar la nueva clase
    g_tank = new Transformer();
    g_env = new Environment();
    g_hud = new HUD();
    g_hud->windowW = g_winW;
    g_hud->windowH = g_winH;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);

    std::cout << "=== Tank Transformer ===\n"
        << "T: Transformar   W/S: Caminar/Parar   A/D: Torreta\n"
        << "Q/E: Canon       H: Escotilla          G: Saludar\n"
        << "F: Disparar      C: Color cuerpo       X: Seleccionar parte\n"
        << "Flechas: Mover entorno   I/J/K/L: Camara   ESC: Salir\n";

    glutMainLoop();
    delete g_tank; delete g_env; delete g_hud;
    return 0;
}