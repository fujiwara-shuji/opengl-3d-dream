#include "Ray.h"
#include "../utils/Utils.h"
#include <cmath>

namespace RayIntersection {

    bool isPointInsideTriangle(const Vector3& point, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& normal) {
        // Left-hand side test for each edge (from CLAUDE.md algorithm)

        // Edge AB (v0 → v1)
        Vector3 edge1 = v1 - v0;
        Vector3 toPoint1 = point - v0;
        Vector3 cross1 = Vector3::cross(edge1, toPoint1);
        if (Vector3::dot(cross1, normal) < 0) return false;

        // Edge BC (v1 → v2)
        Vector3 edge2 = v2 - v1;
        Vector3 toPoint2 = point - v1;
        Vector3 cross2 = Vector3::cross(edge2, toPoint2);
        if (Vector3::dot(cross2, normal) < 0) return false;

        // Edge CA (v2 → v0)
        Vector3 edge3 = v0 - v2;
        Vector3 toPoint3 = point - v2;
        Vector3 cross3 = Vector3::cross(edge3, toPoint3);
        if (Vector3::dot(cross3, normal) < 0) return false;

        return true; // All edges passed left-hand side test
    }

    bool intersectPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal, float& distance, Vector3& hitPoint) {
        float denom = Vector3::dot(planeNormal, ray.direction);

        // Check if ray is parallel to plane
        if (std::abs(denom) < 1e-6f) {
            return false;
        }

        Vector3 toPlane = planePoint - ray.origin;
        distance = Vector3::dot(toPlane, planeNormal) / denom;

        // Check if intersection is behind ray origin
        if (distance < 0) {
            return false;
        }

        hitPoint = ray.origin + ray.direction * distance;
        return true;
    }

    TriangleHit intersectTriangle(const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3& v2) {
        TriangleHit result;
        result.hit = false;

        // Calculate triangle normal (Left-hand winding = CCW = front face)
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 normal = Vector3::cross(edge1, edge2).normalized();

        // Face orientation test
        float denom = Vector3::dot(normal, ray.direction);
        bool isFrontFace = (denom < 0);  // Negative means front face

        if (std::abs(denom) < 1e-6f) {
            return result; // Ray is parallel to triangle
        }

        // Plane intersection
        Vector3 toPlane = v0 - ray.origin;
        float t = Vector3::dot(toPlane, normal) / denom;
        if (t < 0) {
            return result; // Intersection behind ray origin
        }

        Vector3 point = ray.origin + ray.direction * t;

        // Triangle interior test
        if (!isPointInsideTriangle(point, v0, v1, v2, normal)) {
            return result;
        }

        // Valid intersection found
        result.hit = true;
        result.distance = t;
        result.point = point;
        result.normal = isFrontFace ? normal : -normal;  // Always point towards ray origin
        result.isFrontFace = isFrontFace;

        return result;
    }

}