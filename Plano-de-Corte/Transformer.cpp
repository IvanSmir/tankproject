#include "Transformer.h"

#ifdef _WIN32
#include <windows.h>
#include <vector>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

const float Transformer::SPEED = 0.55f;
static const wchar_t* SND_SHOOT = L"sounds\\shoot.wav";

// =============================================================================
// Constructor + build
// =============================================================================
Transformer::Transformer() : root("transformer") {
    build();
    applyPose(TransformMode::HUMANOID);
}

void Transformer::build() {
    body = new Box("body");
    torso = new Box("torso");
    head = new Box("head");
    armL = new Cylinder("armL", 10);
    armR = new Cylinder("armR", 10);
    legL = new Box("legL");
    legR = new Box("legR");

    root.addChild(body);
    root.addChild(torso);
    root.addChild(head);
    root.addChild(armL);
    root.addChild(armR);
    root.addChild(legL);
    root.addChild(legR);

    // Ruedas
    float wx[] = { -1.1f, 1.1f };
    float wz[] = { -1.6f, 1.4f };
    int wIdx = 0;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            auto* w = new Cylinder("wheel" + std::to_string(wIdx), 16);
            w->color = COL_WHEEL;
            w->scale = { 0.65f, 0.40f, 0.65f };
            w->rotation = { 0, 0, 90 };
            w->position = { wx[i], 0.35f, wz[j] };
            w->visible = false;
            root.addChild(w);
            wheels[wIdx++] = w;
        }
    }

    // Helice
    propeller = new Cylinder("propeller", 12);
    propeller->position = { 0, 1.2f, -2.1f };
    propeller->scale = { 2.0f, 0.1f, 0.2f };
    propeller->rotation = { 90, 0, 0 };
    propeller->color = COL_ANT;
    propeller->visible = false;
    root.addChild(propeller);

    // Timon
    rudder = new Box("rudder");
    rudder->position = { 0, 0.4f, 2.6f };
    rudder->scale = { 0.1f, 0.8f, 1.0f };
    rudder->color = COL_TRACK;
    rudder->visible = false;
    root.addChild(rudder);

    // Antena y faro
    antenna = new Cylinder("antenna", 6);
    antenna->color = COL_ANT;
    antenna->scale = { 0.06f, 1.0f, 0.06f };
    antenna->position = { -0.5f, 1.3f, 0.3f };
    antenna->visible = false;
    root.addChild(antenna);

    headlight = new Sphere("headlight");
    headlight->color = COL_ANT;
    headlight->scale = { 0.16f, 0.16f, 0.16f };
    headlight->position = { 0, 0.55f, -1.85f };
    headlight->visible = false;
    root.addChild(headlight);

    // Luces de saludo (visibles en todos los modos)
    lightL = new Sphere("lightL");
    lightL->color = COL_ANT;
    lightL->scale = { 0.14f, 0.14f, 0.14f };
    lightL->position = { -0.35f, 1.6f, 0.18f };
    lightL->visible = true;
    root.addChild(lightL);

    lightR = new Sphere("lightR");
    lightR->color = COL_ANT;
    lightR->scale = { 0.14f, 0.14f, 0.14f };
    lightR->position = { 0.35f, 1.6f, 0.18f };
    lightR->visible = true;
    root.addChild(lightR);
}

// =============================================================================
// SISTEMA DE SELECCION Y MOVIMIENTO DE PARTES (Req. 2)
// =============================================================================

// Nombres legibles para el HUD
std::string Transformer::selectedPartName() const {
    switch (selectedPart) {
    case SelectedPart::ARM_L:    return "Brazo Izq / Ala Izq";
    case SelectedPart::ARM_R:    return "Brazo Der / Ala Der";
    case SelectedPart::LEG_L:    return "Pierna Izq / Oruga Izq";
    case SelectedPart::LEG_R:    return "Pierna Der / Oruga Der";
    case SelectedPart::HEAD:     return "Cabeza / Escotilla";
    case SelectedPart::TORSO:    return "Torso / Torreta";
    case SelectedPart::WHEEL_0:  return "Rueda 0 (Izq-Tras)";
    case SelectedPart::WHEEL_1:  return "Rueda 1 (Der-Tras)";
    case SelectedPart::WHEEL_2:  return "Rueda 2 (Izq-Del)";
    case SelectedPart::WHEEL_3:  return "Rueda 3 (Der-Del)";
    case SelectedPart::PROPELLER:return "Helice";
    case SelectedPart::RUDDER:   return "Timon";
    default:                     return "Ninguna";
    }
}

// Obtener puntero al componente seleccionado
Component* Transformer::getSelectedComponent() {
    switch (selectedPart) {
    case SelectedPart::ARM_L:    return armL;
    case SelectedPart::ARM_R:    return armR;
    case SelectedPart::LEG_L:    return legL;
    case SelectedPart::LEG_R:    return legR;
    case SelectedPart::HEAD:     return head;
    case SelectedPart::TORSO:    return torso;
    case SelectedPart::WHEEL_0:  return wheels[0];
    case SelectedPart::WHEEL_1:  return wheels[1];
    case SelectedPart::WHEEL_2:  return wheels[2];
    case SelectedPart::WHEEL_3:  return wheels[3];
    case SelectedPart::PROPELLER:return propeller;
    case SelectedPart::RUDDER:   return rudder;
    default:                     return nullptr;
    }
}

// Ciclar adelante con TAB
void Transformer::selectNextPart() {
    int next = ((int)selectedPart + 1) % (int)SelectedPart::COUNT;
    // Saltar ruedas si no son visibles
    SelectedPart np = (SelectedPart)next;
    if (np >= SelectedPart::WHEEL_0 && np <= SelectedPart::WHEEL_3) {
        if (!wheels[0] || !wheels[0]->visible) {
            selectedPart = SelectedPart::PROPELLER;
            updateSelectionHighlight(); return;
        }
    }
    if (np == SelectedPart::PROPELLER && propeller && !propeller->visible) {
        selectedPart = SelectedPart::RUDDER;
        updateSelectionHighlight(); return;
    }
    if (np == SelectedPart::RUDDER && rudder && !rudder->visible) {
        selectedPart = SelectedPart::NONE;
        updateSelectionHighlight(); return;
    }
    selectedPart = np;
    updateSelectionHighlight();
}

// Ciclar atras con Shift+TAB
void Transformer::selectPrevPart() {
    int prev = ((int)selectedPart - 1 + (int)SelectedPart::COUNT) % (int)SelectedPart::COUNT;
    selectedPart = (SelectedPart)prev;
    updateSelectionHighlight();
}

// Mover la parte seleccionada en un eje (req. 2 + req. 5)
// axis: 0=rotX, 1=rotY, 2=rotZ   delta: grados a rotar
void Transformer::moveSelectedPart(int axis, float delta) {
    Component* c = getSelectedComponent();
    if (!c) return;

    switch (axis) {
    case 0: c->rotation.x += delta; break;
    case 1: c->rotation.y += delta; break;
    case 2: c->rotation.z += delta; break;
    }

    // Limites logicos por parte
    if (selectedPart == SelectedPart::HEAD)
        c->rotation.x = clampF(c->rotation.x, -90.f, 30.f);
    if (selectedPart == SelectedPart::TORSO)
        c->rotation.y = clampF(c->rotation.y, -180.f, 180.f);
    if (selectedPart == SelectedPart::ARM_R || selectedPart == SelectedPart::ARM_L)
        c->rotation.x = clampF(c->rotation.x, -150.f, 150.f);
    if (selectedPart == SelectedPart::PROPELLER) {
        // Sin limite, gira libremente
    }
}

// Resaltar (amarillo) la parte seleccionada, deseleccionar las demas
void Transformer::updateSelectionHighlight() {
    // Limpiar todas
    auto clear = [](Component* c) { if (c) c->selected = false; };
    clear(body); clear(torso); clear(head);
    clear(armL); clear(armR);
    clear(legL); clear(legR);
    for (int i = 0; i < 4; i++) clear(wheels[i]);
    clear(propeller); clear(rudder);

    // Marcar la seleccionada
    Component* sel = getSelectedComponent();
    if (sel) sel->selected = true;
}

// =============================================================================
// shoot
// =============================================================================
void Transformer::shoot() {
    if (shootCooldown > 0) return;
    shootCooldown = 0.5f;

    if (currentMode == TransformMode::PLANE) {
        // Dos bombas desde las alas
        for (int side = -1; side <= 1; side += 2) {
            Bullet b;
            b.pos = { root.position.x + side * 1.8f,
                       root.position.y + 1.2f,
                       root.position.z };
            b.dir = { 0.f, -0.5f, -1.0f };
            b.life = 2.5f;
            b.active = true;
            b.isAir = true;
            bullets.push_back(b);
        }
        recoilTimer = 0.25f;
    }
    else {
        Bullet b;
        b.pos = { root.position.x, root.position.y + 0.8f, root.position.z - 2.4f };
        b.dir = { 0, 0, -1 };
        b.life = 2.0f;
        b.active = true;
        b.isAir = false;
        bullets.push_back(b);
    }
    playSound(SND_SHOOT);
}

// =============================================================================
// spawnExplosion
// =============================================================================
void Transformer::spawnExplosion(Vec3 pos) {
    for (int i = 0; i < 12; ++i) {
        Particle p;
        p.pos = pos;
        p.life = 0.5f + (i % 3) * 0.1f;
        p.maxLife = p.life;
        float angle = (float)i / 12.f * 2.f * 3.14159f;
        float speed = 1.5f + (i % 4) * 0.5f;
        p.vel = { cosf(angle) * speed, fabsf(sinf(angle)) * speed * 0.8f + 0.5f, sinf(angle) * speed };
        p.color = i < 4 ? Vec3{ 1.f,0.6f,0.f } : i < 8 ? Vec3{ 1.f,0.2f,0.f } : Vec3{ 0.5f,0.5f,0.5f };
        p.active = true;
        particles.push_back(p);
    }
}

// =============================================================================
// update
// =============================================================================
void Transformer::update(float dt) {
    // Transformacion
    if (transforming) {
        transformT += dt * SPEED;
        if (transformT >= 1.f) { transformT = 1.f; transforming = false; currentMode = targetMode; }
        float s = smoothstep(transformT);
        apply(body, lerp2(srcPose.body, dstPose.body, s));
        apply(torso, lerp2(srcPose.torso, dstPose.torso, s));
        apply(head, lerp2(srcPose.head, dstPose.head, s));
        apply(armL, lerp2(srcPose.armL, dstPose.armL, s));
        apply(armR, lerp2(srcPose.armR, dstPose.armR, s));
        apply(legL, lerp2(srcPose.legL, dstPose.legL, s));
        apply(legR, lerp2(srcPose.legR, dstPose.legR, s));
        bool half = (s > 0.5f);
        for (int i = 0; i < 4; i++) if (wheels[i]) wheels[i]->visible = half ? dstPose.wheelsVis : srcPose.wheelsVis;
        if (antenna)   antenna->visible = half ? dstPose.antVis : srcPose.antVis;
        if (headlight) headlight->visible = half ? dstPose.lightVis : srcPose.lightVis;
        if (propeller) propeller->visible = half ? dstPose.propVis : srcPose.propVis;
        if (rudder)    rudder->visible = half ? dstPose.rudVis : srcPose.rudVis;
        // Actualizar highlight durante transformacion
        updateSelectionHighlight();
    }

    // Caminar
    if (walking) {
        walkPhase += dt * 3.f;
        if (currentMode == TransformMode::HUMANOID) {
            legL->rotation.x = sinf(walkPhase) * 28.f;
            legR->rotation.x = -sinf(walkPhase) * 28.f;
        }
        else if (currentMode == TransformMode::CAR) {
            for (int i = 0; i < 4; i++) if (wheels[i]) wheels[i]->rotation.y += dt * 180.f;
        }
        else if (currentMode == TransformMode::PLANE) {
            if (propeller) propeller->rotation.z += dt * 800.f;
        }
        root.position.z -= dt * 1.4f;
    }

    // Retroceso avion
    if (recoilTimer > 0) {
        recoilTimer -= dt;
        float kick = sinf(recoilTimer * 3.14159f / 0.25f) * 0.08f;
        body->position.z = bodyBasePos.z + kick;
        body->position.y = bodyBasePos.y + kick * 0.5f;
        if (recoilTimer <= 0) { body->position.z = bodyBasePos.z; body->position.y = bodyBasePos.y; }
    }

    // Saludar: torso oscila y brazo/ala se levanta en todos los modos
    if (greeting) {
        greetTimer += dt;
        torso->rotation.y = sinf(greetTimer * 4.f) * 30.f;
        if (currentMode == TransformMode::HUMANOID)
            armL->rotation.z = -65.f + sinf(greetTimer * 5.f) * 50.f;
        else
            armL->rotation.y = sinf(greetTimer * 5.f) * 25.f; // ala oscila en otros modos
        if (greetTimer > 3.f) {
            greeting = false; greetTimer = 0;
            torso->rotation.y = 0;
            armL->rotation.z = -65.f; armL->rotation.y = 0;
        }
    }

    // Hablar: parpadean las tres luces
    if (talking) {
        talkTimer += dt;
        bool blink = fmodf(talkTimer, 0.25f) < 0.12f;
        Vec3 onColor = COL_LIGHT;
        Vec3 offColor = COL_ANT;
        if (headlight) headlight->color = blink ? onColor : offColor;
        if (lightL)    lightL->color = blink ? onColor : offColor;
        if (lightR)    lightR->color = blink ? onColor : offColor;
        if (talkTimer > 4.f) {
            talking = false; talkTimer = 0; lightOn = false;
            // Apagar luces al terminar
            if (headlight) { headlight->color = COL_ANT; headlight->visible = false; }
            if (lightL) { lightL->color = COL_ANT; lightL->visible = true; }
            if (lightR) { lightR->color = COL_ANT; lightR->visible = true; }
        }
    }

    // Proyectiles
    if (shootCooldown > 0) shootCooldown -= dt;
    for (auto& b : bullets) {
        if (!b.active) continue;
        b.pos = b.pos + b.dir * (dt * (b.isAir ? 6.f : 8.f));
        b.life -= dt;
        if (b.isAir && b.pos.y <= 0.f) { spawnExplosion(b.pos); b.active = false; continue; }
        if (b.life <= 0) { if (b.isAir) spawnExplosion(b.pos); b.active = false; }
    }

    // Particulas
    for (auto& p : particles) {
        if (!p.active) continue;
        p.pos = p.pos + p.vel * dt;
        p.vel.y -= 4.f * dt;
        p.life -= dt;
        if (p.life <= 0) p.active = false;
    }
}

// =============================================================================
// draw
// =============================================================================
void Transformer::draw(GLuint shader) {
    root.draw(shader);

    for (auto& b : bullets) {
        if (!b.active) continue;
        if (b.isAir) {
            Cylinder bomb("bomb", 8);
            bomb.color = { 1.f, 0.9f, 0.1f };
            bomb.scale = { 0.12f, 0.35f, 0.12f };
            bomb.position = b.pos;
            bomb.rotation = { 90, 0, 0 };
            bomb.draw(shader);
        }
        else {
            Sphere s("blt");
            s.color = { 1.f, 0.6f, 0.f };
            s.scale = { 0.13f, 0.13f, 0.13f };
            s.position = b.pos;
            s.draw(shader);
        }
    }

    for (auto& p : particles) {
        if (!p.active) continue;
        float ratio = p.life / p.maxLife;
        Sphere spark("spark");
        spark.color = p.color * ratio;
        float sz = 0.08f + (1.f - ratio) * 0.12f;
        spark.scale = { sz, sz, sz };
        spark.position = p.pos;
        spark.draw(shader);
    }
}

// =============================================================================
// Resto de metodos
// =============================================================================
void Transformer::startWalking() { walking = true; playSound(L"sounds\\walk.wav"); }
void Transformer::stopWalking() {
    walking = false;
    legL->rotation.x = legR->rotation.x = 0;
    playSound(NULL);
}
void Transformer::greet() {
    // Saludar funciona en cualquier modo
    greeting = true; greetTimer = 0; talking = true; talkTimer = 0; lightOn = true;
    // Forzar luces visibles al saludar
    if (lightL) { lightL->visible = true; lightL->color = COL_LIGHT; }
    if (lightR) { lightR->visible = true; lightR->color = COL_LIGHT; }
    if (headlight) { headlight->visible = true; headlight->color = COL_LIGHT; }
    playSound(L"greet.wav");
}

void Transformer::playSound(const wchar_t* filename) {
#ifdef _WIN32
    if (!filename) { PlaySound(NULL, NULL, 0); return; }

    std::wstring fn(filename);
    size_t pos = fn.rfind(L'\\');
    if (pos != std::wstring::npos) fn = fn.substr(pos + 1);

    wchar_t exePath[MAX_PATH];
    ZeroMemory(exePath, sizeof(exePath));
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    std::wstring base(exePath);
    size_t sl = base.rfind(L'\\');
    if (sl != std::wstring::npos) base = base.substr(0, sl + 1);

    std::wstring finalPath = base + L"..\\..\\Plano-de-Corte\\sounds\\" + fn;
    PlaySound(finalPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
#endif
}

void Transformer::startTransformTo(TransformMode newMode) {
    if (transforming || currentMode == newMode) return;
    targetMode = newMode;
    transformT = 0.f;
    transforming = true;
    srcPose = getPoseForMode(currentMode);
    dstPose = getPoseForMode(targetMode);
}

void Transformer::rotateTurret(float d) {
    if (currentMode == TransformMode::HUMANOID) {
        armL->rotation.x += d; armR->rotation.x += d;
    }
}
void Transformer::elevateCannon(float d) {
    if (currentMode == TransformMode::CAR) {
        float nx = armR->rotation.x + d;
        armR->rotation.x = nx < 70 ? 70 : nx>110 ? 110 : nx;
    }
}
void Transformer::toggleHatch() {
    static bool open = false; open = !open;
    head->rotation.x = open ? -80.f : 0.f;
}
void Transformer::setBodyColor(Vec3 c) { body->color = c; torso->color = c; }
void Transformer::setTrackColor(Vec3 c) { legL->color = c; legR->color = c; }

void Transformer::apply(Component* c, const PartState& p) {
    if (!c) return;
    c->position = p.pos; c->scale = p.scale; c->rotation = p.rot;
    c->color = p.color;  c->visible = p.visible;
}
void Transformer::applyPose(TransformMode mode) {
    RobotPose p = getPoseForMode(mode);
    apply(body, p.body); apply(torso, p.torso); apply(head, p.head);
    apply(armL, p.armL); apply(armR, p.armR);
    apply(legL, p.legL); apply(legR, p.legR);
    for (int i = 0; i < 4; i++) if (wheels[i]) wheels[i]->visible = p.wheelsVis;
    if (antenna)   antenna->visible = p.antVis;
    if (headlight) headlight->visible = p.lightVis;
    if (propeller) propeller->visible = p.propVis;
    if (rudder)    rudder->visible = p.rudVis;
}
PartState Transformer::lerp2(const PartState& a, const PartState& b, float t) {
    PartState r;
    r.pos = lerpV(a.pos, b.pos, t); r.scale = lerpV(a.scale, b.scale, t);
    r.rot = lerpV(a.rot, b.rot, t); r.color = lerpV(a.color, b.color, t);
    r.visible = a.visible || b.visible; return r;
}
void Transformer::clearSelection(Component* c) {
    if (!c)return; c->selected = false;
    for (auto* ch : c->children) clearSelection(ch);
}
bool Transformer::selectByName(Component* c, const std::string& n) {
    if (!c)return false;
    if (c->name == n) { c->selected = true; return true; }
    for (auto* ch : c->children) if (selectByName(ch, n))return true;
    return false;
}
void Transformer::selectPart(const std::string& n) { clearSelection(&root); selectByName(&root, n); }

RobotPose Transformer::getPoseForMode(TransformMode mode) {
    RobotPose p;
    if (mode == TransformMode::HUMANOID) {
        p.body = { {0,1.1f,0},     {0.55f,0.75f,0.30f},{0,0,0},COL_SHIRT,true };
        p.torso = { {0,1.58f,0},    {0.82f,0.20f,0.32f},{0,0,0},COL_SHIRT,true };
        p.head = { {0,1.98f,0},    {0.36f,0.38f,0.34f},{0,0,0},COL_SKIN, true };
        p.armL = { {-0.50f,1.35f,0},{0.20f,0.68f,0.20f},{0,0,-65},COL_SKIN,true };
        p.armR = { { 0.50f,1.35f,0},{0.20f,0.68f,0.20f},{0,0, 65},COL_SKIN,true };
        p.legL = { {-0.18f,0.52f,0},{0.24f,1.05f,0.24f},{0,0,0},COL_PANTS,true };
        p.legR = { { 0.18f,0.52f,0},{0.24f,1.05f,0.24f},{0,0,0},COL_PANTS,true };
        p.wheelsVis = false; p.antVis = false; p.lightVis = false; p.propVis = false; p.rudVis = false;
    }
    else if (mode == TransformMode::CAR) {
        p.body = { {0,0.45f,0},    {2.0f,0.5f,4.2f},  {0,0,0},COL_GREEN,true };
        p.torso = { {0,0.95f,-0.2f},{1.4f,0.5f,2.0f},  {0,0,0},COL_SHIRT,true };
        p.head = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_SKIN, false };
        p.armL = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_SKIN, false };
        p.armR = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_GREEN,false };
        p.legL = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_TRACK,false };
        p.legR = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_TRACK,false };
        p.wheelsVis = true; p.antVis = true; p.lightVis = true; p.propVis = false; p.rudVis = false;
    }
    else if (mode == TransformMode::PLANE) {
        p.body = { {0,1.2f,0},      {1.0f,0.8f,4.0f}, {0,0,0},COL_SHIRT,true };
        p.torso = { {0,1.8f,-0.5f},  {0.8f,0.6f,1.5f}, {0,0,0},COL_SKIN, true };
        p.head = { {0,1.98f,0},     {0.01f,0.01f,0.01f},{0,0,0},COL_SKIN,false };
        p.armL = { {-2.0f,1.2f,0},  {4.0f,0.1f,1.0f}, {0,0,0},COL_GREEN,true };
        p.armR = { { 2.0f,1.2f,0},  {4.0f,0.1f,1.0f}, {0,0,0},COL_GREEN,true };
        p.legL = { {-0.8f,1.2f,1.5f},{1.5f,0.1f,0.8f},{0,0,0},COL_PANTS,true };
        p.legR = { { 0.8f,1.2f,1.5f},{1.5f,0.1f,0.8f},{0,0,0},COL_PANTS,true };
        p.wheelsVis = false; p.antVis = false; p.lightVis = false; p.propVis = true; p.rudVis = false;
    }
    else if (mode == TransformMode::BOAT) {
        p.body = { {0,0.4f,0},     {2.0f,0.8f,5.0f},  {0,0,0},COL_PANTS,true };
        p.torso = { {0,1.3f,1.0f},  {1.2f,1.0f,1.5f},  {0,0,0},COL_SHIRT,true };
        p.head = { {0,2.0f,1.0f},  {0.3f,0.3f,0.3f},  {0,0,0},COL_SKIN, true };
        p.armL = { {-1.3f,0.3f,0}, {0.4f,0.6f,5.0f},  {0,0,0},COL_GREEN,true };
        p.armR = { { 1.3f,0.3f,0}, {0.4f,0.6f,5.0f},  {0,0,0},COL_GREEN,true };
        p.legL = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_TRACK,false };
        p.legR = { {0,0,0},        {0.01f,0.01f,0.01f},{0,0,0},COL_TRACK,false };
        p.wheelsVis = false; p.antVis = true; p.lightVis = true; p.propVis = false; p.rudVis = true;
    }
    return p;
}