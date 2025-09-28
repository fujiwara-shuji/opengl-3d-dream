#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"

// Forward declarations
struct Ray;

class Camera {
private:
    // Camera position (calculated from target, distance, pitch, yaw)
    mutable Vector3 position;

    // Camera parameters
    Vector3 target;        // Look-at point
    float distance;        // Distance from target
    float pitch;          // Vertical rotation (radians)
    float yaw;            // Horizontal rotation (radians)
    Vector3 up;           // Up vector (fixed to Z-up)

    // Projection parameters
    float fov;            // Field of view (radians)
    float aspectRatio;    // Width / Height
    float nearPlane;      // Near clipping plane
    float farPlane;       // Far clipping plane

    // Cached matrices
    mutable Matrix4 viewMatrix;
    mutable Matrix4 projectionMatrix;
    mutable bool viewDirty;
    mutable bool projectionDirty;

public:
    Camera();
    ~Camera() = default;

    // Camera control
    void setTarget(const Vector3& newTarget);
    void setDistance(float newDistance);
    void setPitch(float newPitch);
    void setYaw(float newYaw);
    void orbit(float deltaPitch, float deltaYaw);
    void zoom(float factor);

    // Preset views
    void setFrontView();   // View from front (1 key)
    void setRightView();   // View from right (3 key)
    void setTopView();     // View from top (7 key)
    void setIsometricView();  // 3/4 view

    // Focus on point
    void focusOnPoint(const Vector3& point, float margin = 2.0f);

    // Projection settings
    void setFOV(float fovDegrees);
    void setAspectRatio(float ratio);
    void setClippingPlanes(float near, float far);

    // Matrix generation
    const Matrix4& getViewMatrix() const;
    const Matrix4& getProjectionMatrix() const;
    Matrix4 getViewProjectionMatrix() const;

    // Getters
    const Vector3& getPosition() const { updatePosition(); return position; }
    const Vector3& getTarget() const { return target; }
    float getDistance() const { return distance; }
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getFOV() const { return fov; }
    float getAspectRatio() const { return aspectRatio; }

    // Utility
    Vector3 getForwardVector() const;
    Vector3 getRightVector() const;
    Vector3 getUpVector() const;

    // Ray casting
    Ray screenToWorldRay(double screenX, double screenY, int windowWidth, int windowHeight) const;

private:
    void updatePosition() const;
    void invalidateView();
    void invalidateProjection();
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;

    // Clamp angles to valid ranges
    float clampPitch(float p) const;
    float normalizeYaw(float y) const;
};