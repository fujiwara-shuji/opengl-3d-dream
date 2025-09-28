#pragma once

#include "../math/Vector3.h"
#include <iostream>

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

// Intersection result structures
struct TriangleHit {
    bool hit = false;
    float distance = 0.0f;
    Vector3 point;
    Vector3 normal;
    bool isFrontFace = true;

    TriangleHit() = default;
    TriangleHit(bool hit, float distance, const Vector3& point, const Vector3& normal, bool isFrontFace)
        : hit(hit), distance(distance), point(point), normal(normal), isFrontFace(isFrontFace) {}
};

// Ray-Triangle intersection functions
namespace RayIntersection {
    // Ray-triangle intersection (using the algorithm from CLAUDE.md)
    TriangleHit intersectTriangle(const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3& v2);

    // Point-in-triangle test (using left-hand side test from CLAUDE.md)
    bool isPointInsideTriangle(const Vector3& point, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& normal);

    // Ray-plane intersection
    bool intersectPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal, float& distance, Vector3& hitPoint);
}