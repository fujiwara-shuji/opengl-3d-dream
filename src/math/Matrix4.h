#pragma once

#include "Vector3.h"
#include <cmath>
#include <iostream>

class Matrix4 {
private:
    float m[16];  // Column-major order (OpenGL standard)

public:
    // Constructors
    Matrix4();
    Matrix4(float diagonal);
    Matrix4(const float* values);

    // Copy constructor and assignment
    Matrix4(const Matrix4& other) = default;
    Matrix4& operator=(const Matrix4& other) = default;

    // Element access
    float& operator[](int index) { return m[index]; }
    const float& operator[](int index) const { return m[index]; }

    float& operator()(int row, int col) { return m[col * 4 + row]; }
    const float& operator()(int row, int col) const { return m[col * 4 + row]; }

    const float* data() const { return m; }
    float* data() { return m; }

    // Matrix operations
    Matrix4 operator+(const Matrix4& other) const;
    Matrix4 operator-(const Matrix4& other) const;
    Matrix4 operator*(const Matrix4& other) const;
    Matrix4 operator*(float scalar) const;

    Matrix4& operator+=(const Matrix4& other);
    Matrix4& operator-=(const Matrix4& other);
    Matrix4& operator*=(const Matrix4& other);
    Matrix4& operator*=(float scalar);

    // Vector transformation
    Vector3 operator*(const Vector3& vec) const;
    Vector3 transformPoint(const Vector3& point) const;
    Vector3 transformDirection(const Vector3& direction) const;

    // Matrix properties
    Matrix4 transposed() const;
    Matrix4 inverse() const;
    float determinant() const;

    void transpose();
    bool invert();

    // Static factory methods
    static Matrix4 identity();
    static Matrix4 translation(const Vector3& translation);
    static Matrix4 rotationX(float angleRadians);
    static Matrix4 rotationY(float angleRadians);
    static Matrix4 rotationZ(float angleRadians);
    static Matrix4 rotation(const Vector3& axis, float angleRadians);
    static Matrix4 scale(const Vector3& scale);
    static Matrix4 scale(float uniformScale);

    // Camera matrices
    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
    static Matrix4 perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane);
    static Matrix4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    // Utility methods
    void setIdentity();
    Vector3 getTranslation() const;
    void setTranslation(const Vector3& translation);

    // Debug
    void print() const;
};

// Global operators
Matrix4 operator*(float scalar, const Matrix4& matrix);
std::ostream& operator<<(std::ostream& os, const Matrix4& matrix);