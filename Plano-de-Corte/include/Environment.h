#pragma once
// Environment.h - Camara orbital y suelo
#include "Primitives.h"
#include <cmath>

class Environment {
public:
    float camYaw    =  45.f;
    float camPitch  = -35.f;
    float camDist   =  18.f;
    Vec3  camTarget = { 0.f, 1.0f, 0.f };
    bool  dragging = false;
    int   lastMX   = 0, lastMY = 0;

    void drawFloor(GLuint shader) {
        static Box tile("tile");
        for (int x = -10; x <= 10; ++x)
            for (int z = -10; z <= 10; ++z) {
                tile.position = { x * 1.f, -0.05f, z * 1.f };
                tile.scale    = { 0.98f, 0.08f, 0.98f };
                bool dark = (x + z) % 2 == 0;
                tile.color = dark ? Vec3{ 0.25f, 0.28f, 0.22f }
                                  : Vec3{ 0.40f, 0.44f, 0.35f };
                tile.draw(shader);
            }
    }

    Mat4 viewMatrix() const {
        float yr = toRad(camYaw);
        float pr = toRad(camPitch);
        Vec3 eye = {
            camTarget.x + camDist * cosf(pr) * sinf(yr),
            camTarget.y + camDist * sinf(-pr),
            camTarget.z + camDist * cosf(pr) * cosf(yr)
        };
        return mat4LookAt(eye, camTarget, { 0, 1, 0 });
    }

    void moveTarget(float dx, float dz) {
        float yr = toRad(camYaw);
        float fx = sinf(yr), fz = cosf(yr);
        float rx = cosf(yr), rz = -sinf(yr);
        camTarget.x += rx * dx + fx * dz;
        camTarget.z += rz * dx + fz * dz;
    }

    void onMouseButton(int btn, int state, int x, int y) {
        if (btn == GLUT_LEFT_BUTTON) {
            dragging = (state == GLUT_DOWN);
            lastMX = x; lastMY = y;
        }
        if (btn == 3) camDist = camDist > 4.f  ? camDist - 0.8f : 4.f;
        if (btn == 4) camDist = camDist < 40.f ? camDist + 0.8f : 40.f;
    }

    void onMouseMotion(int x, int y) {
        if (!dragging) return;
        camYaw   += (x - lastMX) * 0.4f;
        camPitch += (y - lastMY) * 0.3f;
        if (camPitch < -80) camPitch = -80;
        if (camPitch >  -5) camPitch =  -5;
        lastMX = x; lastMY = y;
    }
};
