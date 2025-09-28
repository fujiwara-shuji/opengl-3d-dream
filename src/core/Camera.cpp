#include "Camera.h"
#include "Ray.h"
#include "../utils/Utils.h"
#include <cmath>
#include <algorithm>

Camera::Camera()
    : position(0, -5, 0)  // Start behind the scene looking forward along +Y
    , target(0, 0, 0)
    , distance(5.0f)
    , pitch(0.0f)
    , yaw(0.0f)  // 0 degrees = looking along +Y axis (front view)
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
    Utils::logInfo("CAMERA ORBIT CALLED: deltaPitch=" + std::to_string(deltaPitch) +
                   " deltaYaw=" + std::to_string(deltaYaw));
    pitch = clampPitch(pitch + deltaPitch);
    yaw = normalizeYaw(yaw + deltaYaw);
    invalidateView();
}

void Camera::zoom(float factor) {
    float oldDistance = distance;
    distance = std::max(0.1f, distance * factor);
    Utils::logInfo("CAMERA ZOOM CALLED: factor=" + std::to_string(factor) +
                   " oldDist=" + std::to_string(oldDistance) +
                   " newDist=" + std::to_string(distance));
    invalidateView();
}

void Camera::setFrontView() {
    // Front view: looking along +Y axis (X=right, Z=up)
    pitch = 0.0f;
    yaw = 0.0f;
    invalidateView();
    Utils::logInfo("Camera set to front view: pitch=0, yaw=0");
}

void Camera::setRightView() {
    // Right view: looking along -X axis (Y=forward/depth, Z=up)
    pitch = 0.0f;
    yaw = 90.0f * Utils::DEG_TO_RAD;  // Positive 90 degrees
    invalidateView();
    Utils::logInfo("Camera set to right view");
}

void Camera::setTopView() {
    // Top view: looking down along -Z axis (X=right, Y=forward)
    pitch = -89.0f * Utils::DEG_TO_RAD;  // Negative pitch looks down from above
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
    return Vector3::cross(forward, right); // Right-hand rule: forward × right = up
}

void Camera::updatePosition() const {
    // Convert spherical coordinates to Cartesian
    // In our Z-up, right-hand coordinate system:
    // - Yaw rotates around Z axis (horizontal rotation)
    // - Pitch rotates from horizontal plane (vertical rotation)
    // - X axis points right, Y axis points forward (into screen), Z axis points up

    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);

    // Calculate position relative to target
    // Right-hand Z-up: X=right, Y=forward, Z=up
    float x = distance * cosPitch * sinYaw;
    float y = distance * cosPitch * cosYaw;   // Positive Y is forward (away from camera)
    float z = distance * sinPitch;

    Vector3 newPosition = target + Vector3(x, y, z);

    // Debug: Log when camera moves unexpectedly (distance corruption or extreme values)
    if ((newPosition - position).length() > 0.001f) {
        if (distance < 0.1f || distance > 100.0f ||
            pitch < -95.0f * Utils::DEG_TO_RAD || pitch > 95.0f * Utils::DEG_TO_RAD) {
            Utils::logInfo("CAMERA CORRUPTION DETECTED: pitch=" + std::to_string(pitch * 180.0f / 3.14159f) +
                          " yaw=" + std::to_string(yaw * 180.0f / 3.14159f) +
                          " dist=" + std::to_string(distance));
        }
    }

    position = newPosition;
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

Ray Camera::screenToWorldRay(double screenX, double screenY, int windowWidth, int windowHeight) const {
    // Convert screen coordinates to normalized device coordinates (-1 to 1)
    float x = (2.0f * static_cast<float>(screenX) / windowWidth) - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(screenY) / windowHeight);

    // Get view-projection matrix inverse
    Matrix4 viewProjMatrix = getProjectionMatrix() * getViewMatrix();
    Matrix4 invViewProj = viewProjMatrix.inverse();

    // Points in normalized device coordinates
    Vector3 nearPoint(x, y, -1.0f);  // Near plane
    Vector3 farPoint(x, y, 1.0f);    // Far plane

    // Transform to world coordinates
    Vector3 worldNear = invViewProj * nearPoint;
    Vector3 worldFar = invViewProj * farPoint;

    // Create ray
    Vector3 direction = (worldFar - worldNear).normalized();
    return Ray(getPosition(), direction);
}