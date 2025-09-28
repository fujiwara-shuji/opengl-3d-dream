#pragma once

#include "../math/Vector3.h"
#include <iostream>
#include <limits>

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

// Vertex intersection result
struct VertexHit {
    bool hit = false;
    float distance = 0.0f;
    Vector3 point;
    int vertexIndex = -1;

    VertexHit() = default;
    VertexHit(bool hit, float distance, const Vector3& point, int vertexIndex)
        : hit(hit), distance(distance), point(point), vertexIndex(vertexIndex) {}
};

// Edge intersection result
struct EdgeHit {
    bool hit = false;
    float distance = 0.0f;
    Vector3 point;
    int edgeIndex = -1;
    float edgeParameter = 0.0f;  // Position along edge (0.0 to 1.0)

    EdgeHit() = default;
    EdgeHit(bool hit, float distance, const Vector3& point, int edgeIndex, float edgeParameter)
        : hit(hit), distance(distance), point(point), edgeIndex(edgeIndex), edgeParameter(edgeParameter) {}
};

// Combined raycast result for Model intersection
enum class RaycastResultType {
    NONE,
    VERTEX,
    EDGE,
    FACE
};

struct RaycastResult {
    RaycastResultType type = RaycastResultType::NONE;
    float distance = std::numeric_limits<float>::max();
    Vector3 point;
    int elementIndex = -1;  // Index of vertex, edge, or face

    // Type-specific data
    TriangleHit triangleHit;
    VertexHit vertexHit;
    EdgeHit edgeHit;

    RaycastResult() = default;
};

// Forward declaration for Model class
class Model;

// Ray intersection functions
namespace RayIntersection {
    // Ray-triangle intersection (using the algorithm from CLAUDE.md)
    TriangleHit intersectTriangle(const Ray& ray, const Vector3& v0, const Vector3& v1, const Vector3& v2);

    // Point-in-triangle test (using left-hand side test from CLAUDE.md)
    bool isPointInsideTriangle(const Vector3& point, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& normal);

    // Ray-plane intersection
    bool intersectPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal, float& distance, Vector3& hitPoint);

    // Ray-vertex intersection (distance check with threshold)
    VertexHit intersectVertex(const Ray& ray, const Vector3& vertex, float threshold, int vertexIndex);

    // Ray-edge intersection (closest approach distance check)
    EdgeHit intersectEdge(const Ray& ray, const Vector3& edgeStart, const Vector3& edgeEnd, float threshold, int edgeIndex);

    // Calculate distance between ray and point
    float rayPointDistance(const Ray& ray, const Vector3& point, float& rayParameter);

    // Calculate distance between ray and line segment (from CLAUDE.md algorithm)
    float rayEdgeDistance(const Ray& ray, const Vector3& edgeStart, const Vector3& edgeEnd, float& rayParameter, float& edgeParameter);

    // Visibility checking for selection (occlusion test)
    bool isVertexVisible(const Vector3& cameraPos, const Vector3& vertex, const Model& model);

    // Combined model intersection (finds closest hit among vertices, edges, faces)
    RaycastResult findClosestIntersection(const Ray& ray, const Model& model, float vertexThreshold, float edgeThreshold);
}