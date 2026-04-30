#pragma once
// HUD.h - Texto 2D sobre la escena
#include <GL/freeglut.h>
#include <string>

class HUD {
public:
    int windowW = 800, windowH = 600;

    void draw(bool walking, bool talking, bool lightOn,
              bool isTank, const std::string& selectedPart,
              float shootCD)
    {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowW, 0, windowH);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        drawText(10, windowH - 24, "TANK TRANSFORMER", 1.0f, 0.85f, 0.1f);

        std::string modo = isTank ? "TANQUE" : "HUMANOIDE";
        drawText(10, windowH - 46, "Modo: " + modo, 0.7f, 1.0f, 0.5f);

        int yOff = 68;
        if (walking)  { drawText(10, windowH - yOff, "[CAMINANDO]", 0.4f, 1.0f, 0.4f); yOff += 22; }
        if (talking)  { drawText(10, windowH - yOff, "[HABLANDO]",  1.0f, 0.9f, 0.3f); yOff += 22; }
        if (lightOn)  { drawText(10, windowH - yOff, "[LUZ ON]",    1.0f, 1.0f, 0.2f); yOff += 22; }
        if (!selectedPart.empty())
            { drawText(10, windowH - yOff, "Seleccionado: " + selectedPart, 0.3f, 0.8f, 1.0f); yOff += 22; }
        if (shootCD > 0)
            { drawText(10, windowH - yOff, "Recargando...", 1.0f, 0.4f, 0.2f); }

        int y = 10, dy = 18;
        drawText(10, y + dy * 11, "-- CONTROLES --",             0.8f, 0.8f, 0.8f);
        drawText(10, y + dy * 10, "T:     Transformar",          0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 9,  "W/S:   Caminar / Parar",      0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 8,  "A/D:   Rotar torreta",        0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 7,  "Q/E:   Elevar canon",         0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 6,  "H:     Abrir/cerrar escotilla", 0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 5,  "G:     Saludar y hablar",     0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 4,  "F:     Disparar (tanque)",    0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 3,  "C:     Cambiar color cuerpo", 0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 2,  "X:     Seleccionar parte",    0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 1,  "Flechas: Mover entorno",      0.7f, 0.7f, 0.7f);
        drawText(10, y + dy * 0,  "Drag/IJKL: Rotar camara",    0.7f, 0.7f, 0.7f);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
    }

private:
    void drawText(int x, int y, const std::string& text,
                  float r, float g, float b) {
        glColor3f(r, g, b);
        glRasterPos2i(x, y);
        for (char c : text)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
};
