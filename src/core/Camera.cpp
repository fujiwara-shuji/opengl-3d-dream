#include "Camera.h"
#include "../utils/Utils.h"
#include <cmath>
#include <algorithm>

Camera::Camera()
    : position(0, -5, 0)  // Start behind the scene looking forward
    , target(0, 0, 0)
    , distance(5.0f)
    , pitch(0.0f)
    , yaw(0.0f)  // 0 degrees = looking along +Y axis
    , up(0, 0, 1)  // Z-up coordinate system
    , fov(45.0f * Utils::DEG_TO_RAD)
    , aspectRatio(16.0f / 9.0f)
    , nearPlane(0.1f)
    , farPlane(100.0f)
    , viewDirty(true)
    , projectionDirty(true) {
    updatePosition();
}

void Camera::setTarget(const Vector3& newTarget) {
    target = newTarget;
    invalidateView();
}

void Camera::setDistance(float newDistance) {
    distance = std::max(0.1f, newDistance);  // Minimum distance
    invalidateView();
}

void Camera::setPitch(float newPitch) {
    pitch = clampPitch(newPitch);
    invalidateView();
}

void Camera::setYaw(float newYaw) {
    yaw = normalizeYaw(newYaw);
    invalidateView();
}

void Camera::orbit(float deltaPitch, float deltaYaw) {
    pitch = clampPitch(pitch + deltaPitch);
    yaw = normalizeYaw(yaw + deltaYaw);
    invalidateView();
}

void Camera::zoom(float factor) {
    distance = std::max(0.1f, distance * factor);
    invalidateView();
}

void Camera::setFrontView() {
    pitch = 0.0f;
    yaw = 0.0f;
    invalidateView();
    Utils::logInfo("Camera set to front view");
}

void Camera::setRightView() {
    pitch = 0.0f;
    yaw = -90.0f * Utils::DEG_TO_RAD;
    invalidateView();
    Utils::logInfo("Camera set to right view");
}

void Camera::setTopView() {
    pitch = -89.0f * Utils::DEG_TO_RAD;  // Almost straight down
    yaw = 0.0f;
    invalidateView();
    Utils::logInfo("Camera set to top view");
}

void Camera::setIsometricView() {
    pitch = -30.0f * Utils::DEG_TO_RAD;  // Classic isometric angle
    yaw = 45.0f * Utils::DEG_TO_RAD;
    // Don't modify distance here - keep it as is
    invalidateView();
    Utils::logInfo("Camera set to isometric view");
}

void Camera::focusOnPoint(const Vector3& point, float margin) {
    target = point;
    distance = margin;
    invalidateView();
    Utils::logInfo("Camera focused on point: " + std::to_string(point.x) + ", " +
                   std::to_string(point.y) + ", " + std::to_string(point.z));
}

void Camera::setFOV(float fovDegrees) {
    fov = fovDegrees * Utils::DEG_TO_RAD;
    invalidateProjection();
}

void Camera::setAspectRatio(float ratio) {
    aspectRatio = ratio;
    invalidateProjection();
}

void Camera::setClippingPlanes(float near, float far) {
    nearPlane = near;
    farPlane = far;
    invalidateProjection();
}

const Matrix4& Camera::getViewMatrix() const {
    if (viewDirty) {
        updateViewMatrix();
        viewDirty = false;
    }
    return viewMatrix;
}

const Matrix4& Camera::getProjectionMatrix() const {
    if (projectionDirty) {
        updateProjectionMatrix();
        projectionDirty = false;
    }
    return projectionMatrix;
}

Matrix4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

Vector3 Camera::getForwardVector() const {
    updatePosition();
    return (target - position).normalized();
}

Vector3 Camera::getRightVector() const {
    Vector3 forward = getForwardVector();
    return Vector3::cross(forward, up).normalized();
}

Vector3 Camera::getUpVector() const {
    Vector3 forward = getForwardVector();
    Vector3 right = getRightVector();
    return Vector3::cross(right, forward);
}

void Camera::updatePosition() const {
    // Convert spherical coordinates to Cartesian
    // In our Z-up system:
    // - Yaw rotates around Z axis (horizontal rotation)
    // - Pitch rotates from horizontal plane (vertical rotation)

    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);

    // Calculate position relative to target
    // Note: In Z-up system with Y pointing into screen
    float x = distance * cosPitch * sinYaw;
    float y = -distance * cosPitch * cosYaw;  // Negative for proper orientation
    float z = distance * sinPitch;

    position = target + Vector3(x, y, z);
}

void Camera::invalidateView() {
    viewDirty = true;
    updatePosition();
}

void Camera::invalidateProjection() {
    projectionDirty = true;
}

void Camera::updateViewMatrix() const {
    updatePosition();
    viewMatrix = Matrix4::lookAt(position, target, up);
}

void Camera::updateProjectionMatrix() const {
    projectionMatrix = Matrix4::perspective(fov, aspectRatio, nearPlane, farPlane);
}

float Camera::clampPitch(float p) const {
    // Clamp pitch to avoid gimbal lock
    const float maxPitch = 89.0f * Utils::DEG_TO_RAD;
    return std::clamp(p, -maxPitch, maxPitch);
}

float Camera::normalizeYaw(float y) const {
    // Normalize yaw to [-PI, PI]
    while (y > Utils::PI) y -= 2.0f * Utils::PI;
    while (y < -Utils::PI) y += 2.0f * Utils::PI;
    return y;
}