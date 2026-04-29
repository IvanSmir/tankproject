#pragma once
// Tank.h - Clase principal del Transformer
#include "Primitives.h"
#include <cmath>
#include <string>
#include <vector>

static const Vec3 COL_GREEN{ 0.22f, 0.35f, 0.13f };
static const Vec3 COL_TRACK{ 0.12f, 0.12f, 0.12f };
static const Vec3 COL_WHEEL{ 0.25f, 0.22f, 0.18f };
static const Vec3 COL_ANT  { 0.60f, 0.60f, 0.60f };
static const Vec3 COL_LIGHT{ 1.00f, 0.90f, 0.30f };
static const Vec3 COL_SKIN { 0.85f, 0.65f, 0.45f };
static const Vec3 COL_SHIRT{ 0.20f, 0.25f, 0.65f };
static const Vec3 COL_PANTS{ 0.15f, 0.18f, 0.40f };

inline float lerpF(float a, float b, float t) { return a + (b - a) * t; }
inline Vec3  lerpV(Vec3 a, Vec3 b, float t) {
    return { lerpF(a.x, b.x, t), lerpF(a.y, b.y, t), lerpF(a.z, b.z, t) };
}
inline float smoothstep(float t) {
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    return t * t * (3.f - 2.f * t);
}

struct PartState {
    Vec3 pos   = { 0, 0, 0 };
    Vec3 scale = { 1, 1, 1 };
    Vec3 rot   = { 0, 0, 0 };
    Vec3 color = { 1, 1, 1 };
    bool visible = true;
};

class Tank {
public:
    Component root;
    Box*      body      = nullptr;
    Box*      torso     = nullptr;
    Box*      head      = nullptr;
    Box*      armL      = nullptr;
    Cylinder* armR      = nullptr;
    Box*      legL      = nullptr;
    Box*      legR      = nullptr;
    Cylinder* wheels[8] = {};
    Cylinder* antenna   = nullptr;
    Sphere*   headlight = nullptr;

    bool  walking  = false;
    bool  lightOn  = false;
    float walkPhase = 0;

    bool  transforming = false;
    bool  isTank       = false;
    float transformT   = 0.f;
    float transformDir = 1.f;
    static const float SPEED;

    Tank() : root("tank") {
        build();
        applyPose(0.f);
    }

    // Poses de cada parte: h=humanoide, k=tanque
    PartState stBody(float t) {
        PartState h, k;
        h.pos = { 0, 1.1f, 0 };   h.scale = { 0.55f, 0.75f, 0.30f }; h.color = COL_SHIRT;
        k.pos = { 0, 0.40f, 0 };  k.scale = { 2.4f, 0.65f, 3.6f };   k.color = COL_GREEN;
        return lerp2(h, k, t);
    }

    PartState stTorso(float t) {
        PartState h, k;
        h.pos = { 0, 1.58f, 0 };  h.scale = { 0.82f, 0.20f, 0.32f }; h.color = COL_SHIRT;
        k.pos = { 0, 0.73f, 0 };  k.scale = { 1.6f, 0.50f, 2.0f };   k.color = COL_GREEN;
        return lerp2(h, k, t);
    }

    PartState stHead(float t) {
        PartState h, k;
        h.pos = { 0, 1.98f, 0 };  h.scale = { 0.36f, 0.38f, 0.34f }; h.color = COL_SKIN;
        h.visible = true;
        k.pos = { 0, 1.98f, 0 };  k.scale = { 0.01f, 0.01f, 0.01f }; k.color = COL_SKIN;
        k.visible = false;
        PartState r = lerp2(h, k, t);
        r.visible = (t < 0.5f);
        return r;
    }

    PartState stArmL(float t) {
        PartState h, k;
        h.pos = { -0.50f, 1.35f, 0 }; h.scale = { 0.20f, 0.68f, 0.20f };
        h.rot = { 0, 0, -65 }; h.color = COL_SKIN; h.visible = true;
        k.pos = { -0.50f, 1.35f, 0 }; k.scale = { 0.01f, 0.01f, 0.01f };
        k.rot = { 0, 0, -65 }; k.color = COL_SKIN; k.visible = false;
        PartState r = lerp2(h, k, t);
        r.visible = (t < 0.5f);
        return r;
    }

    PartState stArmR(float t) {
        PartState h, k;
        h.pos = { 0.50f, 1.35f, 0 }; h.scale = { 0.20f, 0.68f, 0.20f };
        h.rot = { 0, 0, 65 }; h.color = COL_SKIN;
        k.pos = { 0.0f, 0.70f, -2.0f }; k.scale = { 0.20f, 1.40f, 0.20f };
        k.rot = { 90, 0, 0 }; k.color = COL_GREEN;
        return lerp2(h, k, t);
    }

    PartState stLegL(float t) {
        PartState h, k;
        h.pos = { -0.18f, 0.52f, 0 }; h.scale = { 0.24f, 1.05f, 0.24f }; h.color = COL_PANTS;
        k.pos = { -1.35f, 0.22f, 0 }; k.scale = { 0.44f, 0.52f, 3.8f };  k.color = COL_TRACK;
        return lerp2(h, k, t);
    }

    PartState stLegR(float t) {
        PartState h, k;
        h.pos = { 0.18f, 0.52f, 0 }; h.scale = { 0.24f, 1.05f, 0.24f }; h.color = COL_PANTS;
        k.pos = { 1.35f, 0.22f, 0 }; k.scale = { 0.44f, 0.52f, 3.8f };  k.color = COL_TRACK;
        return lerp2(h, k, t);
    }

    void applyPose(float t) {
        float s = smoothstep(t);
        apply(body,  stBody(s));
        apply(torso, stTorso(s));
        apply(head,  stHead(s));
        apply(armL,  stArmL(s));
        apply(armR,  stArmR(s));
        apply(legL,  stLegL(s));
        apply(legR,  stLegR(s));
        for (int i = 0; i < 8; i++) wheels[i]->visible = (t > 0.5f);
        antenna->visible   = (t > 0.4f);
        headlight->visible = (t > 0.4f);
    }

    void build() {
        body  = new Box("body");    root.addChild(body);
        torso = new Box("torso");   root.addChild(torso);
        head  = new Box("head");    root.addChild(head);
        armL  = new Box("armL");    root.addChild(armL);
        armR  = new Cylinder("armR", 10); root.addChild(armR);
        legL  = new Box("legL");    root.addChild(legL);
        legR  = new Box("legR");    root.addChild(legR);

        float wz[] = { -1.5f, -0.5f, 0.5f, 1.5f };
        for (int i = 0; i < 4; ++i) {
            auto* wl = new Cylinder("wL" + std::to_string(i), 12);
            wl->color = COL_WHEEL; wl->scale = { 0.50f, 0.38f, 0.50f };
            wl->rotation = { 0, 0, 90 }; wl->position = { -1.38f, 0.18f, wz[i] };
            wl->visible = false; root.addChild(wl); wheels[i] = wl;

            auto* wr = new Cylinder("wR" + std::to_string(i), 12);
            wr->color = COL_WHEEL; wr->scale = { 0.50f, 0.38f, 0.50f };
            wr->rotation = { 0, 0, 90 }; wr->position = { 1.38f, 0.18f, wz[i] };
            wr->visible = false; root.addChild(wr); wheels[4 + i] = wr;
        }

        antenna = new Cylinder("antenna", 6);
        antenna->color = COL_ANT; antenna->scale = { 0.06f, 1.0f, 0.06f };
        antenna->position = { -0.5f, 1.3f, 0.3f }; antenna->visible = false;
        root.addChild(antenna);

        headlight = new Sphere("headlight");
        headlight->color = COL_ANT; headlight->scale = { 0.16f, 0.16f, 0.16f };
        headlight->position = { 0, 0.55f, -1.85f }; headlight->visible = false;
        root.addChild(headlight);
    }

    void update(float dt) {
        // Animacion de transformacion
        if (transforming) {
            transformT += dt * SPEED * transformDir;
            if (transformT >= 1.f) { transformT = 1.f; transforming = false; isTank = true; }
            if (transformT <= 0.f) { transformT = 0.f; transforming = false; isTank = false; }
            applyPose(transformT);
        }

        // Animacion de caminata
        if (walking) {
            walkPhase += dt * 3.f;
            if (isTank) {
                for (int i = 0; i < 8; i++) wheels[i]->rotation.x += dt * 180.f;
            } else {
                legL->rotation.x =  sinf(walkPhase) * 28.f;
                legR->rotation.x = -sinf(walkPhase) * 28.f;
            }
            root.position.z -= dt * 1.4f;
        }
    }

    void startTransform() {
        if (transforming) return;
        transformDir = isTank ? -1.f : 1.f;
        transforming = true;
    }

    void startWalking() { walking = true; }
    void stopWalking()  { walking = false; legL->rotation.x = legR->rotation.x = 0; }

    void rotateTurret(float d) {
        if (isTank) { torso->rotation.y += d; armR->rotation.y += d; }
    }

    void elevateCannon(float d) {
        if (isTank) {
            float nx = armR->rotation.x + d;
            armR->rotation.x = nx < 70 ? 70 : nx > 110 ? 110 : nx;
        }
    }

    void toggleHatch() {
        static bool open = false;
        open = !open;
        head->rotation.x = open ? -80.f : 0.f;
    }

    void setBodyColor(Vec3 c)  { body->color = c; torso->color = c; }
    void setTrackColor(Vec3 c) { legL->color = c; legR->color = c; }

    void clearSelection(Component* c) {
        c->selected = false;
        for (auto* ch : c->children) clearSelection(ch);
    }

    bool selectByName(Component* c, const std::string& n) {
        if (c->name == n) { c->selected = true; return true; }
        for (auto* ch : c->children)
            if (selectByName(ch, n)) return true;
        return false;
    }

    void selectPart(const std::string& n) {
        clearSelection(&root);
        selectByName(&root, n);
    }

    void draw(GLuint shader) {
        root.draw(shader);
    }

private:
    void apply(Component* c, const PartState& p) {
        c->position = p.pos; c->scale = p.scale;
        c->rotation = p.rot; c->color = p.color;
        c->visible  = p.visible;
    }

    PartState lerp2(const PartState& a, const PartState& b, float t) {
        PartState r;
        r.pos   = lerpV(a.pos,   b.pos,   t);
        r.scale = lerpV(a.scale, b.scale, t);
        r.rot   = lerpV(a.rot,   b.rot,   t);
        r.color = lerpV(a.color, b.color, t);
        r.visible = a.visible;
        return r;
    }
};

const float Tank::SPEED = 0.55f;
