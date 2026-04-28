#pragma once
// Primitives.h - Figuras 3D: Box, Cylinder, Sphere
#include "Component.h"
#include <vector>
#include <cmath>

class Box : public Component {
public:
    explicit Box(const std::string& n = "box") : Component(n) {}

    void render(GLuint, const Mat4&) override {
        static const float v[] = {
            -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
            -0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
             0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
             0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
            -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
            -0.5f,-0.5f,-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
             0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
             0.5f,-0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,
            -0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
            -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
            -0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
        };
        static GLuint vao = 0, vbo = 0;
        if (!vao) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (void*)0);
            glEnableVertexAttribArray(0);
        }
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
};

class Cylinder : public Component {
public:
    int segs;
    explicit Cylinder(const std::string& n = "cylinder", int s = 16)
        : Component(n), segs(s) {}

    void render(GLuint, const Mat4&) override {
        std::vector<float> verts;
        float tp = 2.f * 3.14159265f;
        for (int i = 0; i < segs; ++i) {
            float a0 = (float)i / segs * tp;
            float a1 = (float)(i + 1) / segs * tp;
            float x0 = cosf(a0) * .5f, z0 = sinf(a0) * .5f;
            float x1 = cosf(a1) * .5f, z1 = sinf(a1) * .5f;
            auto p = [&](float x, float y, float z) {
                verts.push_back(x); verts.push_back(y); verts.push_back(z);
            };
            p(x0, -.5f, z0); p(x1, -.5f, z1); p(x1, .5f, z1);
            p(x0, -.5f, z0); p(x1,  .5f, z1); p(x0, .5f, z0);
            p(0, .5f, 0); p(x0, .5f, z0); p(x1, .5f, z1);
            p(0, -.5f, 0); p(x1, -.5f, z1); p(x0, -.5f, z0);
        }
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (void*)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 3));
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};

class Sphere : public Component {
public:
    int stacks, slices;
    explicit Sphere(const std::string& n = "sphere", int st = 10, int sl = 14)
        : Component(n), stacks(st), slices(sl) {}

    void render(GLuint, const Mat4&) override {
        std::vector<float> verts;
        float pi = 3.14159265f;
        for (int i = 0; i < stacks; ++i) {
            float p0 = pi * ((float)i / stacks - .5f);
            float p1 = pi * ((float)(i + 1) / stacks - .5f);
            for (int j = 0; j < slices; ++j) {
                float t0 = 2 * pi * (float)j / slices;
                float t1 = 2 * pi * (float)(j + 1) / slices;
                auto v = [&](float ph, float th) {
                    verts.push_back(.5f * cosf(ph) * cosf(th));
                    verts.push_back(.5f * sinf(ph));
                    verts.push_back(.5f * cosf(ph) * sinf(th));
                };
                v(p0, t0); v(p1, t0); v(p1, t1);
                v(p0, t0); v(p1, t1); v(p0, t1);
            }
        }
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, verts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (void*)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 3));
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};
