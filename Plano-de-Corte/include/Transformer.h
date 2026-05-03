#pragma once
#include "Primitives.h"
#include <cmath>
#include <string>
#include <vector>

// Constantes de color
static const Vec3 COL_GREEN{ 0.22f, 0.35f, 0.13f };
static const Vec3 COL_TRACK{ 0.12f, 0.12f, 0.12f };
static const Vec3 COL_WHEEL{ 0.25f, 0.22f, 0.18f };
static const Vec3 COL_ANT{ 0.60f, 0.60f, 0.60f };
static const Vec3 COL_LIGHT{ 1.00f, 0.90f, 0.30f };
static const Vec3 COL_SKIN{ 0.85f, 0.65f, 0.45f };
static const Vec3 COL_SHIRT{ 0.20f, 0.25f, 0.65f };
static const Vec3 COL_PANTS{ 0.15f, 0.18f, 0.40f };

// Funciones de interpolación matemáticas inline
inline float lerpF(float a, float b, float t) { return a + (b - a) * t; }
inline Vec3  lerpV(Vec3 a, Vec3 b, float t) {
    return { lerpF(a.x, b.x, t), lerpF(a.y, b.y, t), lerpF(a.z, b.z, t) };
}
inline float smoothstep(float t) {
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    return t * t * (3.f - 2.f * t);
}

// Enumerador de los estados de transformación
enum class TransformMode {
    HUMANOID,
    CAR,
    PLANE,
    BOAT
};

// Estructura pura de estados
struct PartState {
    Vec3 pos = { 0, 0, 0 };
    Vec3 scale = { 1, 1, 1 };
    Vec3 rot = { 0, 0, 0 };
    Vec3 color = { 1, 1, 1 };
    bool visible = true;
};

// Estructura para el disparo
struct Bullet {
    Vec3  pos;
    Vec3  dir;
    float life = 2.0f;
    bool  active = false;
};

struct RobotPose {
    PartState body, torso, head, armL, armR, legL, legR;
    bool wheelsVis, antVis, lightVis, propVis, rudVis;
};


class Transformer {
public:
    Component root;

    // --- PIEZAS EXISTENTES ---
    Box* body = nullptr;
    Box* torso = nullptr;
    Box* head = nullptr;
    Cylinder* armL = nullptr;
    Cylinder* armR = nullptr;
    Box* legL = nullptr;
    Box* legR = nullptr;
    Cylinder* wheels[4] = {};
    Cylinder* antenna = nullptr;
    Sphere* headlight = nullptr;

    // --- NUEVAS PIEZAS ---
    Cylinder* propeller = nullptr; // Para el avión
    Box* rudder = nullptr; // Para el barco

    // --- ESTADO Y ANIMACIÓN ---
    TransformMode currentMode = TransformMode::HUMANOID;
    TransformMode targetMode = TransformMode::HUMANOID;

    bool  transforming = false;
    float transformT = 0.f;
    static const float SPEED;

    // --- ACCIONES Y BALAS ---
    bool  walking = false;
    bool  greeting = false;
    bool  talking = false;
    bool  lightOn = false;
    float walkPhase = 0;
    float greetTimer = 0;
    float talkTimer = 0;
    float shootCooldown = 0;
    std::vector<Bullet> bullets;

    // Constructor
    Transformer();

    // Métodos principales
    void build();
    void update(float dt);
    void draw(GLuint shader);
    void startTransformTo(TransformMode newMode);
    void rotateTurret(float d);
    void elevateCannon(float d);
    void toggleHatch();
    void setBodyColor(Vec3 c);
    void setTrackColor(Vec3 c);


    // Acciones del usuario
    void startWalking();
    void stopWalking();
    void shoot();
    void greet();
    void clearSelection(Component* c);
    bool selectByName(Component* c, const std::string& n);
    void selectPart(const std::string& n);

private:
    void apply(Component* c, const PartState& p);
    void applyPose(TransformMode mode);
    PartState lerp2(const PartState& a, const PartState& b, float t);
    void playSound(const wchar_t* filename);
    RobotPose srcPose, dstPose;
    RobotPose getPoseForMode(TransformMode mode);
};