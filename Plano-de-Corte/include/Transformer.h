#pragma once
#include "Primitives.h"
#include <cmath>
#include <string>
#include <vector>

static const Vec3 COL_GREEN{ 0.22f, 0.35f, 0.13f };
static const Vec3 COL_TRACK{ 0.12f, 0.12f, 0.12f };
static const Vec3 COL_WHEEL{ 0.25f, 0.22f, 0.18f };
static const Vec3 COL_ANT{ 0.60f, 0.60f, 0.60f };
static const Vec3 COL_LIGHT{ 1.00f, 0.90f, 0.30f };
static const Vec3 COL_SKIN{ 0.85f, 0.65f, 0.45f };
static const Vec3 COL_SHIRT{ 0.20f, 0.25f, 0.65f };
static const Vec3 COL_PANTS{ 0.15f, 0.18f, 0.40f };

inline float lerpF(float a, float b, float t) { return a + (b - a) * t; }
inline Vec3  lerpV(Vec3 a, Vec3 b, float t) {
    return { lerpF(a.x,b.x,t), lerpF(a.y,b.y,t), lerpF(a.z,b.z,t) };
}
inline float smoothstep(float t) {
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    return t * t * (3.f - 2.f * t);
}

enum class TransformMode { HUMANOID, CAR, PLANE, BOAT };

// Enum de todas las partes movibles
enum class SelectedPart {
    NONE,
    ARM_L, ARM_R,       // brazos / alas
    LEG_L, LEG_R,       // piernas / orugas
    HEAD,               // cabeza / escotilla
    TORSO,              // torso / torreta
    WHEEL_0, WHEEL_1, WHEEL_2, WHEEL_3,  // ruedas
    PROPELLER,          // helice
    RUDDER,             // timon
    COUNT
};

struct PartState {
    Vec3 pos = { 0,0,0 };
    Vec3 scale = { 1,1,1 };
    Vec3 rot = { 0,0,0 };
    Vec3 color = { 1,1,1 };
    bool visible = true;
};

struct Bullet {
    Vec3  pos, dir;
    float life = 2.0f;
    bool  active = false;
    bool  isAir = false;
};

struct Particle {
    Vec3  pos, vel;
    float life = 0.6f, maxLife = 0.6f;
    bool  active = false;
    Vec3  color = { 1.f,0.5f,0.f };
};

struct RobotPose {
    PartState body, torso, head, armL, armR, legL, legR;
    bool wheelsVis, antVis, lightVis, propVis, rudVis;
};

class Transformer {
public:
    Component root;

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
    Sphere* lightL = nullptr;  // luz izquierda (saludo)
    Sphere* lightR = nullptr;  // luz derecha (saludo)
    Cylinder* propeller = nullptr;
    Box* rudder = nullptr;

    TransformMode currentMode = TransformMode::HUMANOID;
    TransformMode targetMode = TransformMode::HUMANOID;

    bool  transforming = false;
    float transformT = 0.f;
    static const float SPEED;

    bool  walking = false, greeting = false, talking = false, lightOn = false;
    float walkPhase = 0, greetTimer = 0, talkTimer = 0, shootCooldown = 0;
    float recoilTimer = 0;
    Vec3  bodyBasePos = { 0,1.2f,0 };

    std::vector<Bullet>   bullets;
    std::vector<Particle> particles;

    // ── Sistema de seleccion y movimiento de partes (req. 2) ────────────────
    SelectedPart selectedPart = SelectedPart::NONE;

    // Devuelve el nombre legible de la parte seleccionada (para HUD)
    std::string selectedPartName() const;

    // Ciclar parte seleccionada con TAB
    void selectNextPart();
    void selectPrevPart();

    // Mover la parte seleccionada: axis 0=X, 1=Y, 2=Z
    void moveSelectedPart(int axis, float delta);

    // Resaltar visualmente la parte seleccionada
    void updateSelectionHighlight();

    // ── Metodos principales ──────────────────────────────────────────────────
    Transformer();
    void build();
    void update(float dt);
    void draw(GLuint shader);
    void startTransformTo(TransformMode newMode);
    void rotateTurret(float d);
    void elevateCannon(float d);
    void toggleHatch();
    void setBodyColor(Vec3 c);
    void setTrackColor(Vec3 c);
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
    void spawnExplosion(Vec3 pos);
    RobotPose srcPose, dstPose;
    RobotPose getPoseForMode(TransformMode mode);

    // Obtener el Component* de la parte actualmente seleccionada
    Component* getSelectedComponent();
};