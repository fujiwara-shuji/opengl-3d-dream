#include "Matrix4.h"
#include "../utils/Utils.h"
#include <cstring>
#include <iomanip>

Matrix4::Matrix4() {
    setIdentity();
}

Matrix4::Matrix4(float diagonal) {
    std::memset(m, 0, sizeof(m));
    m[0] = m[5] = m[10] = m[15] = diagonal;
}

Matrix4::Matrix4(const float* values) {
    std::memcpy(m, values, sizeof(m));
}

Matrix4 Matrix4::operator+(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 16; ++i) {
        result.m[i] = m[i] + other.m[i];
    }
    return result;
}

Matrix4 Matrix4::operator-(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 16; ++i) {
        result.m[i] = m[i] - other.m[i];
    }
    return result;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;

    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result(row, col) = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result(row, col) += (*this)(row, k) * other(k, col);
            }
        }
    }

    return result;
}

Matrix4 Matrix4::operator*(float scalar) const {
    Matrix4 result;
    for (int i = 0; i < 16; ++i) {
        result.m[i] = m[i] * scalar;
    }
    return result;
}

Matrix4& Matrix4::operator+=(const Matrix4& other) {
    for (int i = 0; i < 16; ++i) {
        m[i] += other.m[i];
    }
    return *this;
}

Matrix4& Matrix4::operator-=(const Matrix4& other) {
    for (int i = 0; i < 16; ++i) {
        m[i] -= other.m[i];
    }
    return *this;
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
    *this = *this * other;
    return *this;
}

Matrix4& Matrix4::operator*=(float scalar) {
    for (int i = 0; i < 16; ++i) {
        m[i] *= scalar;
    }
    return *this;
}

Vector3 Matrix4::operator*(const Vector3& vec) const {
    return transformPoint(vec);
}

Vector3 Matrix4::transformPoint(const Vector3& point) const {
    float x = m[0] * point.x + m[4] * point.y + m[8]  * point.z + m[12];
    float y = m[1] * point.x + m[5] * point.y + m[9]  * point.z + m[13];
    float z = m[2] * point.x + m[6] * point.y + m[10] * point.z + m[14];
    float w = m[3] * point.x + m[7] * point.y + m[11] * point.z + m[15];

    if (std::abs(w) > 1e-6f) {
        return Vector3(x / w, y / w, z / w);
    }
    return Vector3(x, y, z);
}

Vector3 Matrix4::transformDirection(const Vector3& direction) const {
    float x = m[0] * direction.x + m[4] * direction.y + m[8]  * direction.z;
    float y = m[1] * direction.x + m[5] * direction.y + m[9]  * direction.z;
    float z = m[2] * direction.x + m[6] * direction.y + m[10] * direction.z;

    return Vector3(x, y, z);
}

Matrix4 Matrix4::transposed() const {
    Matrix4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result(col, row) = (*this)(row, col);
        }
    }
    return result;
}

void Matrix4::transpose() {
    *this = transposed();
}

float Matrix4::determinant() const {
    float a00 = m[0], a01 = m[4], a02 = m[8],  a03 = m[12];
    float a10 = m[1], a11 = m[5], a12 = m[9],  a13 = m[13];
    float a20 = m[2], a21 = m[6], a22 = m[10], a23 = m[14];
    float a30 = m[3], a31 = m[7], a32 = m[11], a33 = m[15];

    return a00 * (a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) + a13 * (a21 * a32 - a22 * a31)) -
           a01 * (a10 * (a22 * a33 - a23 * a32) - a12 * (a20 * a33 - a23 * a30) + a13 * (a20 * a32 - a22 * a30)) +
           a02 * (a10 * (a21 * a33 - a23 * a31) - a11 * (a20 * a33 - a23 * a30) + a13 * (a20 * a31 - a21 * a30)) -
           a03 * (a10 * (a21 * a32 - a22 * a31) - a11 * (a20 * a32 - a22 * a30) + a12 * (a20 * a31 - a21 * a30));
}

Matrix4 Matrix4::inverse() const {
    Matrix4 result;

    float det = determinant();
    if (std::abs(det) < 1e-6f) {
        Utils::logWarning("Matrix is singular, returning identity matrix");
        return identity();
    }

    float invDet = 1.0f / det;

    // Calculate cofactor matrix and transpose (adjugate matrix)
    result.m[0]  = (m[5] * (m[10] * m[15] - m[11] * m[14]) - m[6] * (m[9] * m[15] - m[11] * m[13]) + m[7] * (m[9] * m[14] - m[10] * m[13])) * invDet;
    result.m[1]  = -(m[1] * (m[10] * m[15] - m[11] * m[14]) - m[2] * (m[9] * m[15] - m[11] * m[13]) + m[3] * (m[9] * m[14] - m[10] * m[13])) * invDet;
    result.m[2]  = (m[1] * (m[6] * m[15] - m[7] * m[14]) - m[2] * (m[5] * m[15] - m[7] * m[13]) + m[3] * (m[5] * m[14] - m[6] * m[13])) * invDet;
    result.m[3]  = -(m[1] * (m[6] * m[11] - m[7] * m[10]) - m[2] * (m[5] * m[11] - m[7] * m[9]) + m[3] * (m[5] * m[10] - m[6] * m[9])) * invDet;

    result.m[4]  = -(m[4] * (m[10] * m[15] - m[11] * m[14]) - m[6] * (m[8] * m[15] - m[11] * m[12]) + m[7] * (m[8] * m[14] - m[10] * m[12])) * invDet;
    result.m[5]  = (m[0] * (m[10] * m[15] - m[11] * m[14]) - m[2] * (m[8] * m[15] - m[11] * m[12]) + m[3] * (m[8] * m[14] - m[10] * m[12])) * invDet;
    result.m[6]  = -(m[0] * (m[6] * m[15] - m[7] * m[14]) - m[2] * (m[4] * m[15] - m[7] * m[12]) + m[3] * (m[4] * m[14] - m[6] * m[12])) * invDet;
    result.m[7]  = (m[0] * (m[6] * m[11] - m[7] * m[10]) - m[2] * (m[4] * m[11] - m[7] * m[8]) + m[3] * (m[4] * m[10] - m[6] * m[8])) * invDet;

    result.m[8]  = (m[4] * (m[9] * m[15] - m[11] * m[13]) - m[5] * (m[8] * m[15] - m[11] * m[12]) + m[7] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[9]  = -(m[0] * (m[9] * m[15] - m[11] * m[13]) - m[1] * (m[8] * m[15] - m[11] * m[12]) + m[3] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[10] = (m[0] * (m[5] * m[15] - m[7] * m[13]) - m[1] * (m[4] * m[15] - m[7] * m[12]) + m[3] * (m[4] * m[13] - m[5] * m[12])) * invDet;
    result.m[11] = -(m[0] * (m[5] * m[11] - m[7] * m[9]) - m[1] * (m[4] * m[11] - m[7] * m[8]) + m[3] * (m[4] * m[9] - m[5] * m[8])) * invDet;

    result.m[12] = -(m[4] * (m[9] * m[14] - m[10] * m[13]) - m[5] * (m[8] * m[14] - m[10] * m[12]) + m[6] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[13] = (m[0] * (m[9] * m[14] - m[10] * m[13]) - m[1] * (m[8] * m[14] - m[10] * m[12]) + m[2] * (m[8] * m[13] - m[9] * m[12])) * invDet;
    result.m[14] = -(m[0] * (m[5] * m[14] - m[6] * m[13]) - m[1] * (m[4] * m[14] - m[6] * m[12]) + m[2] * (m[4] * m[13] - m[5] * m[12])) * invDet;
    result.m[15] = (m[0] * (m[5] * m[10] - m[6] * m[9]) - m[1] * (m[4] * m[10] - m[6] * m[8]) + m[2] * (m[4] * m[9] - m[5] * m[8])) * invDet;

    return result;
}

bool Matrix4::invert() {
    Matrix4 inv = inverse();
    if (std::abs(determinant()) < 1e-6f) {
        return false;
    }
    *this = inv;
    return true;
}

Matrix4 Matrix4::identity() {
    return Matrix4(1.0f);
}

Matrix4 Matrix4::translation(const Vector3& translation) {
    Matrix4 result = identity();
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

Matrix4 Matrix4::rotationX(float angleRadians) {
    Matrix4 result = identity();
    float c = std::cos(angleRadians);
    float s = std::sin(angleRadians);

    result.m[5] = c;   result.m[6] = s;
    result.m[9] = -s;  result.m[10] = c;

    return result;
}

Matrix4 Matrix4::rotationY(float angleRadians) {
    Matrix4 result = identity();
    float c = std::cos(angleRadians);
    float s = std::sin(angleRadians);

    result.m[0] = c;   result.m[2] = -s;
    result.m[8] = s;   result.m[10] = c;

    return result;
}

Matrix4 Matrix4::rotationZ(float angleRadians) {
    Matrix4 result = identity();
    float c = std::cos(angleRadians);
    float s = std::sin(angleRadians);

    result.m[0] = c;   result.m[1] = s;
    result.m[4] = -s;  result.m[5] = c;

    return result;
}

Matrix4 Matrix4::rotation(const Vector3& axis, float angleRadians) {
    Vector3 normalizedAxis = axis.normalized();
    float c = std::cos(angleRadians);
    float s = std::sin(angleRadians);
    float oneMinusC = 1.0f - c;

    float x = normalizedAxis.x;
    float y = normalizedAxis.y;
    float z = normalizedAxis.z;

    Matrix4 result = identity();

    result.m[0] = c + x * x * oneMinusC;
    result.m[1] = x * y * oneMinusC + z * s;
    result.m[2] = x * z * oneMinusC - y * s;

    result.m[4] = y * x * oneMinusC - z * s;
    result.m[5] = c + y * y * oneMinusC;
    result.m[6] = y * z * oneMinusC + x * s;

    result.m[8] = z * x * oneMinusC + y * s;
    result.m[9] = z * y * oneMinusC - x * s;
    result.m[10] = c + z * z * oneMinusC;

    return result;
}

Matrix4 Matrix4::scale(const Vector3& scale) {
    Matrix4 result = identity();
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

Matrix4 Matrix4::scale(float uniformScale) {
    return scale(Vector3(uniformScale));
}

Matrix4 Matrix4::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 forward = (target - eye).normalized();
    Vector3 right = Vector3::cross(forward, up).normalized();
    Vector3 newUp = Vector3::cross(right, forward);

    Matrix4 result = identity();

    result.m[0] = right.x;    result.m[4] = right.y;    result.m[8] = right.z;     result.m[12] = -Vector3::dot(right, eye);
    result.m[1] = newUp.x;    result.m[5] = newUp.y;    result.m[9] = newUp.z;     result.m[13] = -Vector3::dot(newUp, eye);
    result.m[2] = -forward.x; result.m[6] = -forward.y; result.m[10] = -forward.z; result.m[14] = Vector3::dot(forward, eye);
    result.m[3] = 0;          result.m[7] = 0;          result.m[11] = 0;          result.m[15] = 1;

    return result;
}

Matrix4 Matrix4::perspective(float fovRadians, float aspectRatio, float nearPlane, float farPlane) {
    float tanHalfFov = std::tan(fovRadians * 0.5f);

    Matrix4 result;
    std::memset(result.m, 0, sizeof(result.m));

    result.m[0] = 1.0f / (aspectRatio * tanHalfFov);
    result.m[5] = 1.0f / tanHalfFov;
    result.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.m[11] = -1.0f;
    result.m[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);

    return result;
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Matrix4 result = identity();

    result.m[0] = 2.0f / (right - left);
    result.m[5] = 2.0f / (top - bottom);
    result.m[10] = -2.0f / (farPlane - nearPlane);
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);

    return result;
}

void Matrix4::setIdentity() {
    std::memset(m, 0, sizeof(m));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

Vector3 Matrix4::getTranslation() const {
    return Vector3(m[12], m[13], m[14]);
}

void Matrix4::setTranslation(const Vector3& translation) {
    m[12] = translation.x;
    m[13] = translation.y;
    m[14] = translation.z;
}

void Matrix4::print() const {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            std::cout << std::setw(10) << std::setprecision(3) << (*this)(row, col) << " ";
        }
        std::cout << std::endl;
    }
}

Matrix4 operator*(float scalar, const Matrix4& matrix) {
    return matrix * scalar;
}

std::ostream& operator<<(std::ostream& os, const Matrix4& matrix) {
    for (int row = 0; row < 4; ++row) {
        os << "[ ";
        for (int col = 0; col < 4; ++col) {
            os << std::setw(8) << std::setprecision(3) << matrix(row, col);
            if (col < 3) os << ", ";
        }
        os << " ]";
        if (row < 3) os << std::endl;
    }
    return os;
}