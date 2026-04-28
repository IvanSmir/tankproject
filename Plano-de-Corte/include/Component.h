#pragma once
// Component.h - Nodo del grafo de escena
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <string>
#include "Math3D.h"

class Component {
public:
    Vec3 position;
    Vec3 rotation;
    Vec3 scale  = {1, 1, 1};
    Vec3 color  = {1, 1, 1};
    bool  selected = false;
    bool  visible  = true;
    std::string name;
    std::vector<Component*> children;
    Component* parent = nullptr;

    Component() = default;
    explicit Component(const std::string& n) : name(n) {}
    virtual ~Component() = default;

    void addChild(Component* child) {
        child->parent = this;
        children.push_back(child);
    }

    Mat4 localMatrix() const {
        Mat4 m = Mat4::identity();
        m = mat4Translate(m, position);
        m = mat4Rotate(m, rotation.z, {0, 0, 1});
        m = mat4Rotate(m, rotation.y, {0, 1, 0});
        m = mat4Rotate(m, rotation.x, {1, 0, 0});
        m = mat4Scale(m, scale);
        return m;
    }

    void draw(GLuint shader, const Mat4& parentMat = Mat4::identity()) {
        if (!visible) return;
        Mat4 world = parentMat * localMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, world.ptr());
        Vec3 dc = selected ? Vec3{1, 1, 0} : color;
        glUniform3f(glGetUniformLocation(shader, "objectColor"), dc.x, dc.y, dc.z);
        render(shader, world);
        for (auto* c : children) c->draw(shader, world);
    }

    virtual void render(GLuint, const Mat4&) {}
    virtual void update(float dt) { for (auto* c : children) c->update(dt); }
};
