#pragma once

#include <cmath>
#include <iostream>

struct Vector3 {
    float x, y, z;

    // Constructors
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(float value) : x(value), y(value), z(value) {}

    // Copy constructor and assignment
    Vector3(const Vector3& other) = default;
    Vector3& operator=(const Vector3& other) = default;

    // Basic arithmetic operations
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(float scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    // Compound assignment operations
    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // Comparison operations
    bool operator==(const Vector3& other) const {
        const float epsilon = 1e-6f;
        return std::abs(x - other.x) < epsilon &&
               std::abs(y - other.y) < epsilon &&
               std::abs(z - other.z) < epsilon;
    }

    bool operator!=(const Vector3& other) const {
        return !(*this == other);
    }

    // Vector operations
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    Vector3 normalized() const {
        float len = length();
        if (len < 1e-6f) {
            return Vector3(0.0f, 0.0f, 0.0f);
        }
        return *this / len;
    }

    void normalize() {
        float len = length();
        if (len >= 1e-6f) {
            *this /= len;
        }
    }

    // Static utility functions
    static float dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static float distance(const Vector3& a, const Vector3& b) {
        return (a - b).length();
    }

    static float distanceSquared(const Vector3& a, const Vector3& b) {
        return (a - b).lengthSquared();
    }

    static Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
        return a + (b - a) * t;
    }

    static Vector3 reflect(const Vector3& incident, const Vector3& normal) {
        return incident - normal * (2.0f * dot(incident, normal));
    }

    // Predefined vectors
    static const Vector3 ZERO;
    static const Vector3 ONE;
    static const Vector3 UNIT_X;
    static const Vector3 UNIT_Y;
    static const Vector3 UNIT_Z;
    static const Vector3 UP;
    static const Vector3 RIGHT;
    static const Vector3 FORWARD;
};

// Global operators
inline Vector3 operator*(float scalar, const Vector3& vector) {
    return vector * scalar;
}

// Stream output
inline std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}