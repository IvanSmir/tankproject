#pragma once
#include <cmath>
#include <cstring>

static const float M_PI_VAL = 3.14159265358979323846f;
inline float toRad(float deg) { return deg * M_PI_VAL / 180.0f; }
inline float clampF(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s)       const { return { x * s,   y * s,   z * s }; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }

    float  dot(const Vec3& o)  const { return x * o.x + y * o.y + z * o.z; }
    Vec3   cross(const Vec3& o)const { return { y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x }; }
    float  len()               const { return sqrtf(x * x + y * y + z * z); }
    Vec3   norm()              const { float l = len(); return l > 0 ? Vec3{ x / l,y / l,z / l } : Vec3{}; }
};

struct Mat4 {
    float m[16] = {};

    static Mat4 identity() {
        Mat4 r; r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.f; return r;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 r;
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row) {
                float s = 0;
                for (int k = 0; k < 4; ++k)
                    s += m[k * 4 + row] * b.m[col * 4 + k];
                r.m[col * 4 + row] = s;
            }
        return r;
    }

    const float* ptr() const { return m; }
};

inline Mat4 mat4Translate(const Mat4& in, const Vec3& t) {
    Mat4 r = in;
    r.m[12] = in.m[0] * t.x + in.m[4] * t.y + in.m[8] * t.z + in.m[12];
    r.m[13] = in.m[1] * t.x + in.m[5] * t.y + in.m[9] * t.z + in.m[13];
    r.m[14] = in.m[2] * t.x + in.m[6] * t.y + in.m[10] * t.z + in.m[14];
    r.m[15] = in.m[3] * t.x + in.m[7] * t.y + in.m[11] * t.z + in.m[15];
    return r;
}

inline Mat4 mat4Scale(const Mat4& in, const Vec3& s) {
    Mat4 r = in;
    r.m[0] *= s.x;  r.m[1] *= s.x;  r.m[2] *= s.x;  r.m[3] *= s.x;
    r.m[4] *= s.y;  r.m[5] *= s.y;  r.m[6] *= s.y;  r.m[7] *= s.y;
    r.m[8] *= s.z;  r.m[9] *= s.z;  r.m[10] *= s.z; r.m[11] *= s.z;
    return r;
}

inline Mat4 mat4Rotate(const Mat4& in, float angleDeg, const Vec3& axis) {
    float a = toRad(angleDeg);
    Vec3  n = axis.norm();
    float c = cosf(a), s = sinf(a), t = 1.f - c;
    float x = n.x, y = n.y, z = n.z;

    Mat4 rot = Mat4::identity();
    rot.m[0] = t * x * x + c;   rot.m[4] = t * x * y - s * z; rot.m[8] = t * x * z + s * y;
    rot.m[1] = t * x * y + s * z; rot.m[5] = t * y * y + c;   rot.m[9] = t * y * z - s * x;
    rot.m[2] = t * x * z - s * y; rot.m[6] = t * y * z + s * x; rot.m[10] = t * z * z + c;

    return in * rot;
}

inline Mat4 mat4Perspective(float fovDeg, float aspect, float zNear, float zFar) {
    float f = 1.0f / tanf(toRad(fovDeg) * 0.5f);
    Mat4 r;
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zFar + zNear) / (zNear - zFar);
    r.m[11] = -1.f;
    r.m[14] = (2.f * zFar * zNear) / (zNear - zFar);
    return r;
}

inline Mat4 mat4LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye).norm();
    Vec3 s = f.cross(up).norm();
    Vec3 u = s.cross(f);

    Mat4 r = Mat4::identity();
    r.m[0] = s.x;  r.m[4] = s.y;  r.m[8] = s.z;  r.m[12] = -s.dot(eye);
    r.m[1] = u.x;  r.m[5] = u.y;  r.m[9] = u.z;  r.m[13] = -u.dot(eye);
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z; r.m[14] = f.dot(eye);
    r.m[15] = 1.f;
    return r;
}
