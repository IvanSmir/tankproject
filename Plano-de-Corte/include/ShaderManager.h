#pragma once
// ShaderManager.h - Compilacion y enlace de shaders GLSL
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ShaderManager {
public:
    static GLuint load(const char* vertPath, const char* fragPath) {
        std::string vs = readFile(vertPath);
        std::string fs = readFile(fragPath);
        if (vs.empty() || fs.empty()) {
            std::cerr << "[ShaderManager] No se pudo leer shader: "
                      << vertPath << " / " << fragPath << "\n";
            return 0;
        }
        return link(compile(GL_VERTEX_SHADER, vs.c_str()),
                     compile(GL_FRAGMENT_SHADER, fs.c_str()));
    }

    static GLuint loadFromSrc(const char* vsSrc, const char* fsSrc) {
        return link(compile(GL_VERTEX_SHADER, vsSrc),
                     compile(GL_FRAGMENT_SHADER, fsSrc));
    }

private:
    static std::string readFile(const char* path) {
        std::ifstream f(path);
        if (!f.is_open()) return "";
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static GLuint compile(GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
            std::cerr << "[Shader compile error]\n" << log << "\n";
        }
        return s;
    }

    static GLuint link(GLuint vert, GLuint frag) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);
        GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[512]; glGetProgramInfoLog(prog, 512, nullptr, log);
            std::cerr << "[Shader link error]\n" << log << "\n";
        }
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }
};
