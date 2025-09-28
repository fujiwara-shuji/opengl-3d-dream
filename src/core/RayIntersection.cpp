#include "Ray.h"
#include "Model.h"
#include "../utils/Utils.h"
#include <cmath>
#include <algorithm>

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

    float rayPointDistance(const Ray& ray, const Vector3& point, float& rayParameter) {
        // Calculate closest point on ray to the given point
        Vector3 toPoint = point - ray.origin;
        rayParameter = Vector3::dot(toPoint, ray.direction);

        // Clamp to ray (not line) - can't go behind origin
        rayParameter = std::max(rayParameter, 0.0f);

        Vector3 closestOnRay = ray.origin + ray.direction * rayParameter;
        return (point - closestOnRay).length();
    }

    VertexHit intersectVertex(const Ray& ray, const Vector3& vertex, float threshold, int vertexIndex) {
        VertexHit result;

        float rayParam;
        float distance = rayPointDistance(ray, vertex, rayParam);

        if (distance <= threshold) {
            result.hit = true;
            result.distance = rayParam;
            result.point = vertex;
            result.vertexIndex = vertexIndex;
        }

        return result;
    }

    float rayEdgeDistance(const Ray& ray, const Vector3& edgeStart, const Vector3& edgeEnd, float& rayParameter, float& edgeParameter) {
        // Implementation of ray-edge distance calculation from CLAUDE.md
        Vector3 P0 = ray.origin;
        Vector3 d = ray.direction;
        Vector3 A = edgeStart;
        Vector3 B = edgeEnd;
        Vector3 AB = B - A;

        // Check if edge has zero length
        float edgeLengthSq = AB.lengthSquared();
        if (edgeLengthSq < 1e-6f) {
            // Degenerate edge - treat as point
            return rayPointDistance(ray, edgeStart, rayParameter);
        }

        // Step 1: Find closest point on infinite line AB to ray origin
        Vector3 P0A = A - P0;
        float t = Vector3::dot(P0A, AB) / edgeLengthSq;

        // Step 2: Clamp t to edge bounds [0, 1]
        edgeParameter = std::clamp(t, 0.0f, 1.0f);

        // Step 3: Calculate closest point on edge
        Vector3 closestOnEdge = A + AB * edgeParameter;

        // Step 4: Find closest point on ray to this edge point
        Vector3 P0ToClosest = closestOnEdge - P0;
        rayParameter = Vector3::dot(P0ToClosest, d);

        // Step 5: Clamp ray parameter (can't go behind origin)
        rayParameter = std::max(rayParameter, 0.0f);

        Vector3 closestOnRay = P0 + d * rayParameter;

        // Step 6: Return distance
        return (closestOnRay - closestOnEdge).length();
    }

    EdgeHit intersectEdge(const Ray& ray, const Vector3& edgeStart, const Vector3& edgeEnd, float threshold, int edgeIndex) {
        EdgeHit result;

        float rayParam, edgeParam;
        float distance = rayEdgeDistance(ray, edgeStart, edgeEnd, rayParam, edgeParam);

        if (distance <= threshold) {
            result.hit = true;
            result.distance = rayParam;
            result.point = edgeStart + (edgeEnd - edgeStart) * edgeParam;
            result.edgeIndex = edgeIndex;
            result.edgeParameter = edgeParam;
        }

        return result;
    }

    bool isVertexVisible(const Vector3& cameraPos, const Vector3& vertex, const class Model& model) {
        // Create ray from camera to vertex
        Vector3 direction = (vertex - cameraPos).normalized();
        float targetDistance = (vertex - cameraPos).length();
        Ray visibilityRay(cameraPos, direction);

        // Check if any face occludes this vertex
        const auto& faces = model.getFaces();
        const auto& vertices = model.getVertices();

        for (size_t i = 0; i < faces.size(); ++i) {
            const auto& face = faces[i];

            // Skip invalid faces
            if (face.v1 >= vertices.size() || face.v2 >= vertices.size() || face.v3 >= vertices.size()) {
                continue;
            }

            const Vector3& v0 = vertices[face.v1].position;
            const Vector3& v1 = vertices[face.v2].position;
            const Vector3& v2 = vertices[face.v3].position;

            TriangleHit hit = intersectTriangle(visibilityRay, v0, v1, v2);

            // If ray hits a face closer than the target vertex, it's occluded
            if (hit.hit && hit.distance < (targetDistance - 0.001f)) {
                return false;
            }
        }

        return true; // No occlusion found
    }

    RaycastResult findClosestIntersection(const Ray& ray, const class Model& model, float vertexThreshold, float edgeThreshold) {
        RaycastResult result;
        float closestDistance = std::numeric_limits<float>::max();

        const auto& vertices = model.getVertices();
        const auto& edges = model.getEdges();
        const auto& faces = model.getFaces();

        // Check vertex intersections
        for (size_t i = 0; i < vertices.size(); ++i) {
            VertexHit vertexHit = intersectVertex(ray, vertices[i].position, vertexThreshold, static_cast<int>(i));

            if (vertexHit.hit && vertexHit.distance < closestDistance) {
                closestDistance = vertexHit.distance;
                result.type = RaycastResultType::VERTEX;
                result.distance = vertexHit.distance;
                result.point = vertexHit.point;
                result.elementIndex = vertexHit.vertexIndex;
                result.vertexHit = vertexHit;
            }
        }

        // Check edge intersections
        for (size_t i = 0; i < edges.size(); ++i) {
            const auto& edge = edges[i];

            // Skip invalid edges
            if (edge.v1 >= vertices.size() || edge.v2 >= vertices.size()) {
                continue;
            }

            const Vector3& edgeStart = vertices[edge.v1].position;
            const Vector3& edgeEnd = vertices[edge.v2].position;

            EdgeHit edgeHit = intersectEdge(ray, edgeStart, edgeEnd, edgeThreshold, static_cast<int>(i));

            if (edgeHit.hit && edgeHit.distance < closestDistance) {
                closestDistance = edgeHit.distance;
                result.type = RaycastResultType::EDGE;
                result.distance = edgeHit.distance;
                result.point = edgeHit.point;
                result.elementIndex = edgeHit.edgeIndex;
                result.edgeHit = edgeHit;
            }
        }

        // Check face intersections
        for (size_t i = 0; i < faces.size(); ++i) {
            const auto& face = faces[i];

            // Skip invalid faces
            if (face.v1 >= vertices.size() || face.v2 >= vertices.size() || face.v3 >= vertices.size()) {
                continue;
            }

            const Vector3& v0 = vertices[face.v1].position;
            const Vector3& v1 = vertices[face.v2].position;
            const Vector3& v2 = vertices[face.v3].position;

            TriangleHit triangleHit = intersectTriangle(ray, v0, v1, v2);

            if (triangleHit.hit && triangleHit.distance < closestDistance) {
                closestDistance = triangleHit.distance;
                result.type = RaycastResultType::FACE;
                result.distance = triangleHit.distance;
                result.point = triangleHit.point;
                result.elementIndex = static_cast<int>(i);
                result.triangleHit = triangleHit;
            }
        }

        return result;
    }

}