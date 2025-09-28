#pragma once

#include "../math/Vector3.h"

struct Ray {
    Vector3 origin;
    Vector3 direction;

    // Constructors
    Ray() : origin(Vector3::ZERO), direction(Vector3::FORWARD) {}
    Ray(const Vector3& origin, const Vector3& direction)
        : origin(origin), direction(direction.normalized()) {}

    // Copy constructor and assignment
    Ray(const Ray& other) = default;
    Ray& operator=(const Ray& other) = default;

    // Get point along the ray at parameter t
    Vector3 getPoint(float t) const {
        return origin + direction * t;
    }

    // Ray operations
    void normalize() {
        direction = direction.normalized();
    }

    // Check if ray is valid (direction is not zero)
    bool isValid() const {
        return direction.lengthSquared() > 1e-6f;
    }

    // Create ray from two points
    static Ray fromPoints(const Vector3& start, const Vector3& end) {
        return Ray(start, (end - start).normalized());
    }

    // Debug output
    void print() const {
        std::cout << "Ray: origin=" << origin << ", direction=" << direction << std::endl;
    }
};