// main.cpp - Tank Transformer
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <chrono>
#include <string>

#include "include/Math3D.h"
#include "include/ShaderManager.h"
#include "Transformer.h"
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

GLuint        g_shader = 0;
Transformer* g_tank = nullptr;
Environment* g_env = nullptr;
HUD* g_hud = nullptr;
int           g_winW = 900, g_winH = 650;

static const Vec3 BODY_COLORS[] = {
    {0.22f,0.35f,0.13f},{0.55f,0.20f,0.12f},
    {0.15f,0.25f,0.55f},{0.65f,0.55f,0.10f},{0.20f,0.20f,0.20f}
};
static int g_colorIdx = 0;
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

    bool isTankMode = (g_tank->currentMode == TransformMode::CAR);
    // Mostrar nombre de parte seleccionada en HUD
    std::string selName = g_tank->selectedPartName();
    g_hud->draw(g_tank->walking, g_tank->talking, g_tank->lightOn,
        isTankMode, selName, g_tank->shootCooldown);

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
    case 'a': g_tank->rotateTurret(-5); break;
    case 'd': g_tank->rotateTurret(5);  break;
    case 'q': g_tank->elevateCannon(-3); break;
    case 'e': g_tank->elevateCannon(3);  break;
    case 'h': g_tank->toggleHatch(); break;
    case 'f': g_tank->shoot(); break;
    case 'g': g_tank->greet(); break;
    case 'c':
        g_colorIdx = (g_colorIdx + 1) % 5;
        g_tank->setBodyColor(BODY_COLORS[g_colorIdx]);
        break;
    case 't':
        if (g_tank->currentMode == TransformMode::HUMANOID) g_tank->startTransformTo(TransformMode::CAR);
        else if (g_tank->currentMode == TransformMode::CAR)      g_tank->startTransformTo(TransformMode::PLANE);
        else if (g_tank->currentMode == TransformMode::PLANE)    g_tank->startTransformTo(TransformMode::BOAT);
        else                                                    g_tank->startTransformTo(TransformMode::HUMANOID);
        break;
        // ── Seleccion y movimiento de partes (Req. 2) ──
    case '\t': g_tank->selectNextPart(); break;        // TAB: siguiente parte
    case 'u':  g_tank->moveSelectedPart(0, -5.f); break; // Rotar X -
    case 'o':  g_tank->moveSelectedPart(0, 5.f); break; // Rotar X +
    case 'y':  g_tank->moveSelectedPart(1, -5.f); break; // Rotar Y -
    case 'n':  g_tank->moveSelectedPart(1, 5.f); break; // Rotar Y +
    case 'z':  g_tank->moveSelectedPart(2, -5.f); break; // Rotar Z -
    case 'b':  g_tank->moveSelectedPart(2, 5.f); break; // Rotar Z +
        // ── Camara ──
    case 'i': g_env->camPitch = g_env->camPitch > -80 ? g_env->camPitch - 2 : -80; break;
    case 'k': g_env->camPitch = g_env->camPitch < -5 ? g_env->camPitch + 2 : -5;  break;
    case 'j': g_env->camYaw -= 3; break;
    case 'l': g_env->camYaw += 3; break;
    case 27:  glutLeaveMainLoop(); break;
    }
}

void specialKeys(int key, int, int) {
    switch (key) {
    case GLUT_KEY_UP:    g_env->moveTarget(0, 0.5f); break;
    case GLUT_KEY_DOWN:  g_env->moveTarget(0, -0.5f); break;
    case GLUT_KEY_LEFT:  g_env->moveTarget(-0.5f, 0);  break;
    case GLUT_KEY_RIGHT: g_env->moveTarget(0.5f, 0);  break;
    }
}

void mouse(int btn, int state, int x, int y) {
    g_env->onMouseButton(btn, state, x, y);
    if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        static int ti = 0;
        static const Vec3 TC[] = { {0.1f,0.1f,0.1f},{0.35f,0.28f,0.18f},{0.5f,0.15f,0.1f} };
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
    if (err != GLEW_OK) { std::cerr << "GLEW: " << glewGetErrorString(err) << "\n"; return -1; }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";

    glEnable(GL_DEPTH_TEST);
    g_shader = ShaderManager::loadFromSrc(VS_SRC, FS_SRC);
    if (!g_shader) { std::cerr << "Shader error\n"; return -1; }

    g_tank = new Transformer();
    g_env = new Environment();
    g_hud = new HUD();
    g_hud->windowW = g_winW; g_hud->windowH = g_winH;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);

    std::cout
        << "=== Tank Transformer ===\n"
        << "T:     Transformar (HUMANO->AUTO->AVION->BARCO)\n"
        << "W/S:   Caminar / Parar\n"
        << "A/D:   Rotar torreta    Q/E: Elevar canon\n"
        << "H:     Escotilla        G: Saludar    F: Disparar\n"
        << "C:     Cambiar color\n"
        << "TAB:   Seleccionar siguiente parte (se resalta en amarillo)\n"
        << "U/O:   Rotar X  |  Y/N: Rotar Y  |  Z/B: Rotar Z\n"
        << "Flechas: Mover camara   I/J/K/L: Orbitar   ESC: Salir\n";

    glutMainLoop();
    delete g_tank; delete g_env; delete g_hud;
    return 0;
}