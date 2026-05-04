#pragma once
#include <GL/freeglut.h>
#include <string>
#include "Transformer.h"

class HUD {
public:
    int windowW = 800, windowH = 600;

    void draw(bool walking, bool talking, bool lightOn,
              TransformMode mode, const std::string& selectedPart,
              float shootCD)
    {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(0);

        int x = 10;
        int y = windowH - 20;
        int dy = 16;

        // Título y modo
        std::string modoStr;
        switch (mode) {
            case TransformMode::HUMANOID: modoStr = "HUMANOIDE"; break;
            case TransformMode::CAR:      modoStr = "AUTO";      break;
            case TransformMode::PLANE:    modoStr = "AVION";     break;
            case TransformMode::BOAT:     modoStr = "BARCO";     break;
        }
        drawText(x, y, "TANK TRANSFORMER",       1.0f, 0.85f, 0.1f); y -= dy;
        drawText(x, y, "Modo: " + modoStr,       1.0f, 1.0f,  0.0f); y -= dy;

        // Estados activos
        if (walking) { drawText(x, y, "[CAMINANDO]", 0.4f, 1.0f, 0.4f); y -= dy; }
        if (talking) { drawText(x, y, "[HABLANDO]",  1.0f, 0.9f, 0.3f); y -= dy; }
        if (lightOn) { drawText(x, y, "[LUZ ON]",    1.0f, 1.0f, 0.2f); y -= dy; }
        if (!selectedPart.empty()) { drawText(x, y, "Sel: " + selectedPart, 0.3f, 0.8f, 1.0f); y -= dy; }
        if (shootCD > 0) { drawText(x, y, "Recargando...", 1.0f, 0.4f, 0.2f); y -= dy; }

        // Controles según modo
        y -= 4;
        drawText(x, y, "-- CONTROLES --",         0.8f, 0.8f, 0.8f); y -= dy;
        drawText(x, y, "T:   Transformar",         0.7f, 0.7f, 0.7f); y -= dy;
        drawText(x, y, "W/S: Avanzar / Parar",     0.7f, 0.7f, 0.7f); y -= dy;

        if (mode == TransformMode::HUMANOID) {
            drawText(x, y, "A/D: Rotar brazos",    0.7f, 0.7f, 0.7f); y -= dy;
            drawText(x, y, "G:   Saludar",         0.7f, 0.7f, 0.7f); y -= dy;
            drawText(x, y, "TAB: Selec. parte",    0.7f, 0.7f, 0.7f); y -= dy;
            drawText(x, y, "U/O/Y/N/Z/B: Rotar",  0.7f, 0.7f, 0.7f); y -= dy;
        }
        if (mode == TransformMode::CAR) {
            drawText(x, y, "C:   Cambiar color",           0.7f, 0.7f, 0.7f); y -= dy;
            drawText(x, y, "Clic der: Color pistas",       0.7f, 0.7f, 0.7f); y -= dy;
        }
        if (mode == TransformMode::PLANE) {
            drawText(x, y, "Q/E: Elevar alas",     0.7f, 0.7f, 0.7f); y -= dy;
            drawText(x, y, "F:   Disparar",        0.7f, 0.7f, 0.7f); y -= dy;
        }
        if (mode == TransformMode::BOAT) {
            drawText(x, y, "A/D: Rotar timon",     0.7f, 0.7f, 0.7f); y -= dy;
        }

        drawText(x, y, "Flechas/IJKL: Camara",     0.7f, 0.7f, 0.7f); y -= dy;
        drawText(x, y, "Scroll: Zoom",              0.7f, 0.7f, 0.7f);

        glEnable(GL_DEPTH_TEST);
    }

private:
    void drawText(int x, int y, const std::string& text,
                  float r, float g, float b) {
        glColor3f(r, g, b);
        glWindowPos2i(x, y);
        for (char c : text)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
};
