#include "Transformer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

const float Transformer::SPEED = 0.55f;

Transformer::Transformer() : root("transformer") {
    build();
    applyPose(TransformMode::HUMANOID);
}

void Transformer::build() {
    // Inicializar jerarquía existente
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

    // --AUTO -- //
    float wx[] = { -1.1f, 1.1f }; // Posiciones en X (Izquierda y Derecha)
    float wz[] = { -1.6f, 1.4f }; // Posiciones en Z (Atrás y Adelante)
    int wIdx = 0;

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            auto* w = new Cylinder("wheel" + std::to_string(wIdx), 16);
            w->color = COL_WHEEL;
            w->scale = { 0.65f, 0.40f, 0.65f }; // Ruedas más grandes
            w->rotation = { 0, 0, 90 };
            w->position = { wx[i], 0.35f, wz[j] };
            w->visible = false;
            root.addChild(w);
            wheels[wIdx++] = w;
        }
    }

    // -- AVION -- //
    propeller = new Cylinder("propeller", 12);
    propeller->position = { 0, 1.2f, -2.1f };
    propeller->scale = { 2.0f, 0.1f, 0.2f }; // Forma de aspa larga
    propeller->rotation = { 90, 0, 0 };
    propeller->color = COL_ANT;
    propeller->visible = false;
    root.addChild(propeller);

    // -- BARCO -- //
    rudder = new Box("rudder");
    rudder->position = { 0, 0.4f, 2.6f };
    rudder->scale = { 0.1f, 0.8f, 1.0f }; // Forma de timón plano
    rudder->color = COL_TRACK;
    rudder->visible = false;
    root.addChild(rudder);
}

void Transformer::startTransformTo(TransformMode newMode) {
    if (transforming || currentMode == newMode) return;

    targetMode = newMode;
    transformT = 0.f;
    transforming = true;

    // Capturar cómo estamos AHORA y a dónde queremos LLEGAR
    srcPose = getPoseForMode(currentMode);
    dstPose = getPoseForMode(targetMode);
}

void Transformer::update(float dt) {
    // 1. Animación de Transformación
    if (transforming) {
        transformT += dt * SPEED;
        if (transformT >= 1.f) {
            transformT = 1.f;
            transforming = false;
            currentMode = targetMode;
        }

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
        if (antenna) antenna->visible = half ? dstPose.antVis : srcPose.antVis;
        if (headlight) headlight->visible = half ? dstPose.lightVis : srcPose.lightVis;
        if (propeller) propeller->visible = half ? dstPose.propVis : srcPose.propVis;
        if (rudder) rudder->visible = half ? dstPose.rudVis : srcPose.rudVis;
    }

    // 2. Animación de Caminar/Avanzar
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
            if (propeller) propeller->rotation.z += dt * 800.f; // Girar hélice rápido
        }
        root.position.z -= dt * 1.4f;
    }

    // 3. Animación de Saludar (Tecla G)
    if (greeting && currentMode == TransformMode::HUMANOID) {
        greetTimer += dt;
        torso->rotation.y = sinf(greetTimer * 4.f) * 30.f; // Gira el torso
        armL->rotation.z = -65.f + sinf(greetTimer * 5.f) * 50.f; // Levanta el brazo
        if (greetTimer > 3.f) {
            greeting = false;
            greetTimer = 0;
            // Restaurar rotaciones base del Humanoide
            torso->rotation.y = 0;
            armL->rotation.z = -65.f;
        }
    }

    // 4. Animación de Hablar y parpadeo de luz
    if (talking) {
        talkTimer += dt;
        bool blink = fmodf(talkTimer, 0.25f) < 0.12f;
        if (headlight && headlight->visible) headlight->color = blink ? COL_LIGHT : COL_ANT;
        if (talkTimer > 4.f) {
            talking = false;
            talkTimer = 0;
            lightOn = false;
            if (headlight) headlight->color = COL_ANT;
        }
    }

    // 5. Cooldown y movimiento de Disparos
    if (shootCooldown > 0) shootCooldown -= dt;

    for (auto& b : bullets) {
        if (!b.active) continue;
        b.pos = b.pos + b.dir * (dt * 8.f);
        b.life -= dt;
        if (b.life <= 0) b.active = false;
    }
}

void Transformer::draw(GLuint shader) {
    root.draw(shader);
    for (auto& b : bullets) {
        if (!b.active) continue;
        Sphere s("bullet");
        s.color = { 1.f, 0.6f, 0.f };
        s.scale = { 0.13f, 0.13f, 0.13f };
        s.position = b.pos;
        s.draw(shader);
    }
}

void Transformer::startWalking() {
    walking = true;
    playSound(L"sounds/walk.wav");
}

void Transformer::stopWalking() {
    walking = false;
    legL->rotation.x = legR->rotation.x = 0;
    playSound(NULL);
}

void Transformer::shoot() {
    if (shootCooldown > 0) return;
    shootCooldown = 0.5f;
    Bullet b;
    b.pos = { root.position.x, 0.8f, root.position.z - 2.4f };
    b.dir = { 0, 0, -1 };
    b.active = true;
    bullets.push_back(b);
    playSound(L"sounds/shoot.wav");
}

void Transformer::greet() {
    // Solo permitimos saludar en forma humanoide
    if (currentMode != TransformMode::HUMANOID) return;
    greeting = true;
    greetTimer = 0;
    talking = true;
    talkTimer = 0;
    lightOn = true;
    playSound(L"sounds/greet.wav");
}

void Transformer::playSound(const wchar_t* filename) {
#ifdef _WIN32
    PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC);
#endif
}

void Transformer::apply(Component* c, const PartState& p) {
    if (!c) return;
    c->position = p.pos;
    c->scale = p.scale;
    c->rotation = p.rot;
    c->color = p.color;
    c->visible = p.visible;
}

void Transformer::applyPose(TransformMode mode) {
    // Aplicación estática (útil para cuando arranca el programa)
    RobotPose p = getPoseForMode(mode);
    apply(body, p.body);
    apply(torso, p.torso);
    apply(head, p.head);
    apply(armL, p.armL);
    apply(armR, p.armR);
    apply(legL, p.legL);
    apply(legR, p.legR);
    for (int i = 0; i < 4; i++) if (wheels[i]) wheels[i]->visible = p.wheelsVis;
    if (antenna) antenna->visible = p.antVis;
    if (headlight) headlight->visible = p.lightVis;
    if (propeller) propeller->visible = p.propVis;
    if (rudder) rudder->visible = p.rudVis;
}

PartState Transformer::lerp2(const PartState& a, const PartState& b, float t) {
    PartState r;
    r.pos = lerpV(a.pos, b.pos, t);
    r.scale = lerpV(a.scale, b.scale, t);
    r.rot = lerpV(a.rot, b.rot, t);
    r.color = lerpV(a.color, b.color, t);
    r.visible = a.visible || b.visible;
    return r;
}

void Transformer::rotateTurret(float d) {
    // Si estás en modo humano, gira los brazos. Si es Auto, lo ignoramos.
    if (currentMode == TransformMode::HUMANOID) {
        armL->rotation.x += d;
        armR->rotation.x += d;
    }
}

void Transformer::elevateCannon(float d) {
    if (currentMode == TransformMode::CAR) {
        float nx = armR->rotation.x + d;
        armR->rotation.x = nx < 70 ? 70 : nx > 110 ? 110 : nx;
    }
}

void Transformer::toggleHatch() {
    static bool open = false;
    open = !open;
    head->rotation.x = open ? -80.f : 0.f;
}

void Transformer::setBodyColor(Vec3 c) {
    body->color = c;
    torso->color = c;
}

void Transformer::setTrackColor(Vec3 c) {
    legL->color = c;
    legR->color = c;
}

void Transformer::clearSelection(Component* c) {
    if (!c) return;
    c->selected = false;
    for (auto* ch : c->children) clearSelection(ch);
}

bool Transformer::selectByName(Component* c, const std::string& n) {
    if (!c) return false;
    if (c->name == n) { c->selected = true; return true; }
    for (auto* ch : c->children)
        if (selectByName(ch, n)) return true;
    return false;
}

void Transformer::selectPart(const std::string& n) {
    clearSelection(&root);
    selectByName(&root, n);
}

RobotPose Transformer::getPoseForMode(TransformMode mode) {
    RobotPose p;
    if (mode == TransformMode::HUMANOID) {
        p.body = { { 0, 1.1f, 0 }, { 0.55f, 0.75f, 0.30f }, {0,0,0}, COL_SHIRT, true };
        p.torso = { { 0, 1.58f, 0 }, { 0.82f, 0.20f, 0.32f }, {0,0,0}, COL_SHIRT, true };
        p.head = { { 0, 1.98f, 0 }, { 0.36f, 0.38f, 0.34f }, {0,0,0}, COL_SKIN, true };
        p.armL = { { -0.50f, 1.35f, 0 }, { 0.20f, 0.68f, 0.20f }, { 0, 0, -65 }, COL_SKIN, true };
        p.armR = { { 0.50f, 1.35f, 0 }, { 0.20f, 0.68f, 0.20f }, { 0, 0, 65 }, COL_SKIN, true };
        p.legL = { { -0.18f, 0.52f, 0 }, { 0.24f, 1.05f, 0.24f }, {0,0,0}, COL_PANTS, true };
        p.legR = { { 0.18f, 0.52f, 0 }, { 0.24f, 1.05f, 0.24f }, {0,0,0}, COL_PANTS, true };
        p.wheelsVis = false; p.antVis = false; p.lightVis = false; p.propVis = false; p.rudVis = false;
    }
    else if (mode == TransformMode::CAR) {
        // Chasis principal del auto
        p.body = { { 0, 0.45f, 0 }, { 2.0f, 0.5f, 4.2f }, {0,0,0}, COL_GREEN, true };

        // Cabina del auto (techo y ventanas)
        p.torso = { { 0, 0.95f, -0.2f }, { 1.4f, 0.5f, 2.0f }, {0,0,0}, COL_SHIRT, true };

        // Ocultamos todo lo demás (brazos, piernas y cabeza humana)
        p.head = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_SKIN, false };
        p.armL = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_SKIN, false };
        p.armR = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_GREEN, false };
        p.legL = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_TRACK, false };
        p.legR = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_TRACK, false };

        p.wheelsVis = true; p.antVis = true; p.lightVis = true; p.propVis = false; p.rudVis = false;
    }
    else if (mode == TransformMode::PLANE) {
        p.body = { { 0, 1.2f, 0 }, { 1.0f, 0.8f, 4.0f }, {0,0,0}, COL_SHIRT, true }; // Fuselaje
        p.torso = { { 0, 1.8f, -0.5f }, { 0.8f, 0.6f, 1.5f }, {0,0,0}, COL_SKIN, true }; // Cabina
        p.head = { { 0, 1.98f, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_SKIN, false };
        p.armL = { { -2.0f, 1.2f, 0 }, { 4.0f, 0.1f, 1.0f }, { 0, 0, 0 }, COL_GREEN, true }; // Ala L
        p.armR = { {  2.0f, 1.2f, 0 }, { 4.0f, 0.1f, 1.0f }, { 0, 0, 0 }, COL_GREEN, true }; // Ala R
        p.legL = { { -0.8f, 1.2f, 1.5f }, { 1.5f, 0.1f, 0.8f }, {0,0,0}, COL_PANTS, true }; // Cola L
        p.legR = { {  0.8f, 1.2f, 1.5f }, { 1.5f, 0.1f, 0.8f }, {0,0,0}, COL_PANTS, true }; // Cola R
        p.wheelsVis = false; p.antVis = false; p.lightVis = false; p.propVis = true; p.rudVis = false;
    }
    else if (mode == TransformMode::BOAT) {
        p.body = { { 0, 0.4f, 0 }, { 2.0f, 0.8f, 5.0f }, {0,0,0}, COL_PANTS, true }; // Casco
        p.torso = { { 0, 1.3f, 1.0f }, { 1.2f, 1.0f, 1.5f }, {0,0,0}, COL_SHIRT, true }; // Puente de mando
        p.head = { { 0, 2.0f, 1.0f }, { 0.3f, 0.3f, 0.3f }, {0,0,0}, COL_SKIN, true }; // Radar
        p.armL = { { -1.3f, 0.3f, 0 }, { 0.4f, 0.6f, 5.0f }, { 0, 0, 0 }, COL_GREEN, true }; // Pontón L
        p.armR = { {  1.3f, 0.3f, 0 }, { 0.4f, 0.6f, 5.0f }, { 0, 0, 0 }, COL_GREEN, true }; // Pontón R
        p.legL = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_TRACK, false };
        p.legR = { { 0, 0, 0 }, { 0.01f, 0.01f, 0.01f }, {0,0,0}, COL_TRACK, false };
        p.wheelsVis = false; p.antVis = true; p.lightVis = true; p.propVis = false; p.rudVis = true;
    }
    return p;
}