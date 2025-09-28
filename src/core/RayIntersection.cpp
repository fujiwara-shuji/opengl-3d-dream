#include "Ray.h"
#include "Model.h"
#include "../utils/Utils.h"
#include <cmath>
#include <algorithm>
#include <string>

namespace RayIntersection
{

    bool isPointInsideTriangle(const Vector3 &point, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &normal)
    {
        // Left-hand side test for each edge (from CLAUDE.md algorithm)

        // Edge AB (v0 → v1)
        Vector3 edge1 = v1 - v0;
        Vector3 toPoint1 = point - v0;
        Vector3 cross1 = Vector3::cross(edge1, toPoint1);
        if (Vector3::dot(cross1, normal) < 0)
            return false;

        // Edge BC (v1 → v2)
        Vector3 edge2 = v2 - v1;
        Vector3 toPoint2 = point - v1;
        Vector3 cross2 = Vector3::cross(edge2, toPoint2);
        if (Vector3::dot(cross2, normal) < 0)
            return false;

        // Edge CA (v2 → v0)
        Vector3 edge3 = v0 - v2;
        Vector3 toPoint3 = point - v2;
        Vector3 cross3 = Vector3::cross(edge3, toPoint3);
        if (Vector3::dot(cross3, normal) < 0)
            return false;

        return true; // All edges passed left-hand side test
    }

    bool intersectPlane(const Ray &ray, const Vector3 &planePoint, const Vector3 &planeNormal, float &distance, Vector3 &hitPoint)
    {
        float denom = Vector3::dot(planeNormal, ray.direction);

        // Check if ray is parallel to plane
        if (std::abs(denom) < 1e-6f)
        {
            return false;
        }

        Vector3 toPlane = planePoint - ray.origin;
        distance = Vector3::dot(toPlane, planeNormal) / denom;

        // Check if intersection is behind ray origin
        if (distance < 0)
        {
            return false;
        }

        hitPoint = ray.origin + ray.direction * distance;
        return true;
    }

    TriangleHit intersectTriangle(const Ray &ray, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2)
    {
        TriangleHit result;
        result.hit = false;

        // Calculate triangle normal (Left-hand winding = CCW = front face)
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 normal = Vector3::cross(edge1, edge2).normalized();

        // Face orientation test
        float denom = Vector3::dot(normal, ray.direction);
        bool isFrontFace = (denom < 0); // Negative means front face

        if (std::abs(denom) < 1e-6f)
        {
            return result; // Ray is parallel to triangle
        }

        // Plane intersection
        Vector3 toPlane = v0 - ray.origin;
        float t = Vector3::dot(toPlane, normal) / denom;
        if (t < 0)
        {
            return result; // Intersection behind ray origin
        }

        Vector3 point = ray.origin + ray.direction * t;

        // Triangle interior test
        if (!isPointInsideTriangle(point, v0, v1, v2, normal))
        {
            return result;
        }

        // Valid intersection found
        result.hit = true;
        result.distance = t;
        result.point = point;
        result.normal = isFrontFace ? normal : -normal; // Always point towards ray origin
        result.isFrontFace = isFrontFace;

        return result;
    }

    float rayPointDistance(const Ray &ray, const Vector3 &point, float &rayParameter)
    {
        // Calculate closest point on ray to the given point
        Vector3 toPoint = point - ray.origin;
        rayParameter = Vector3::dot(toPoint, ray.direction);

        // Clamp to ray (not line) - can't go behind origin
        rayParameter = std::max(rayParameter, 0.0f);

        Vector3 closestOnRay = ray.origin + ray.direction * rayParameter;
        return (point - closestOnRay).length();
    }

    VertexHit intersectVertex(const Ray &ray, const Vector3 &vertex, float threshold, int vertexIndex)
    {
        VertexHit result;

        float rayParam;
        float distance = rayPointDistance(ray, vertex, rayParam);

        if (distance <= threshold)
        {
            result.hit = true;
            result.distance = rayParam;
            result.point = vertex;
            result.vertexIndex = vertexIndex;
        }

        return result;
    }

    float rayEdgeDistance(const Ray &ray, const Vector3 &edgeStart, const Vector3 &edgeEnd, float &rayParameter, float &edgeParameter)
    {
        // Correct ray-edge distance calculation (3D line-line distance)
        Vector3 P0 = ray.origin;
        Vector3 d = ray.direction;
        Vector3 A = edgeStart;
        Vector3 B = edgeEnd;
        Vector3 AB = B - A;

        // Check if edge has zero length
        float edgeLengthSq = AB.lengthSquared();
        if (edgeLengthSq < 1e-6f)
        {
            // Degenerate edge - treat as point
            return rayPointDistance(ray, edgeStart, rayParameter);
        }

        // Vector from ray origin to edge start
        Vector3 P0A = A - P0;

        // Compute the parameters for closest points on both lines
        float a = Vector3::dot(d, d); // Always positive (ray direction is normalized)
        float b = Vector3::dot(d, AB);
        float c = Vector3::dot(AB, AB); // Edge length squared
        float f = Vector3::dot(d, P0A);
        float e = Vector3::dot(AB, P0A);

        float denom = a * c - b * b;

        if (abs(denom) < 1e-6f)
        {
            // Lines are parallel - find closest point on edge to ray origin
            edgeParameter = std::clamp(-e / c, 0.0f, 1.0f);
            Vector3 closestOnEdge = A + AB * edgeParameter;
            Vector3 P0ToClosest = closestOnEdge - P0;
            rayParameter = Vector3::dot(P0ToClosest, d);
            rayParameter = std::max(rayParameter, 0.0f);
            Vector3 closestOnRay = P0 + d * rayParameter;
            return (closestOnRay - closestOnEdge).length();
        }

        // Compute parameters for closest points
        float s = (b * e - c * f) / denom; // Parameter for ray
        float t = (a * e - b * f) / denom; // Parameter for edge

        // Clamp parameters to valid ranges
        rayParameter = std::max(s, 0.0f);          // Ray can't go backward
        edgeParameter = std::clamp(t, 0.0f, 1.0f); // Edge bounded [0,1]

        // If we clamped edge parameter, recompute ray parameter
        if (t != edgeParameter)
        {
            Vector3 clampedEdgePoint = A + AB * edgeParameter;
            Vector3 P0ToClampedEdge = clampedEdgePoint - P0;
            rayParameter = Vector3::dot(P0ToClampedEdge, d);
            rayParameter = std::max(rayParameter, 0.0f);
        }

        // If we clamped ray parameter, recompute edge parameter
        if (s != rayParameter)
        {
            Vector3 clampedRayPoint = P0 + d * rayParameter;
            Vector3 AToClampedRay = clampedRayPoint - A;
            edgeParameter = Vector3::dot(AToClampedRay, AB) / edgeLengthSq;
            edgeParameter = std::clamp(edgeParameter, 0.0f, 1.0f);
        }

        // Compute final closest points and distance
        Vector3 closestOnRay = P0 + d * rayParameter;
        Vector3 closestOnEdge = A + AB * edgeParameter;

        return (closestOnRay - closestOnEdge).length();
    }

    EdgeHit intersectEdge(const Ray &ray, const Vector3 &edgeStart, const Vector3 &edgeEnd, float threshold, int edgeIndex)
    {
        EdgeHit result;

        float rayParam, edgeParam;
        float distance = rayEdgeDistance(ray, edgeStart, edgeEnd, rayParam, edgeParam);

        // Debug for edge 0 occasionally
        static int debugCallCount = 0;
        bool shouldDebugDetail = (debugCallCount % 50000 == 0) && edgeIndex == 0;
        if (shouldDebugDetail)
        {
            Utils::logInfo("DETAIL intersectEdge " + std::to_string(edgeIndex) + ": rawDistance=" + std::to_string(distance) +
                           " threshold=" + std::to_string(threshold) +
                           " rayParam=" + std::to_string(rayParam) +
                           " edgeParam=" + std::to_string(edgeParam));
        }
        debugCallCount++;

        // Always set the calculated values regardless of hit/miss
        result.distance = rayParam;
        result.point = edgeStart + (edgeEnd - edgeStart) * edgeParam;
        result.edgeIndex = edgeIndex;
        result.edgeParameter = edgeParam;

        if (distance <= threshold)
        {
            result.hit = true;

            if (shouldDebugDetail)
            {
                Utils::logInfo("DETAIL intersectEdge " + std::to_string(edgeIndex) + " HIT: point(" +
                               std::to_string(result.point.x) + "," + std::to_string(result.point.y) + "," + std::to_string(result.point.z) + ")");
            }
        }
        else
        {
            result.hit = false;
        }

        return result;
    }

    EdgeHit intersectEdgeScreenSpace(const Ray &ray, const Vector3 &edgeStart, const Vector3 &edgeEnd,
                                     float threshold, int edgeIndex,
                                     const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUp,
                                     float fov, float aspectRatio)
    {
        EdgeHit result;

        // Calculate camera coordinate system
        Vector3 forward = (cameraTarget - cameraPos).normalized();
        Vector3 right = Vector3::cross(forward, cameraUp).normalized();
        Vector3 up = Vector3::cross(right, forward);

        // Project edge endpoints to screen space
        auto projectToScreen = [&](const Vector3 &point) -> Vector3
        {
            Vector3 toPoint = point - cameraPos;
            float z = Vector3::dot(toPoint, forward);
            if (z <= 0.001f)
                return Vector3(0, 0, -1); // Behind camera

            float x = Vector3::dot(toPoint, right);
            float y = Vector3::dot(toPoint, up);

            // Direct projection onto screen plane at z=1 (no FOV correction)
            float screenX = x / z;
            float screenY = y / z;

            return Vector3(screenX, screenY, z);
        };

        Vector3 screenStart = projectToScreen(edgeStart);
        Vector3 screenEnd = projectToScreen(edgeEnd);

        // Check if both points are behind camera
        if (screenStart.z < 0 && screenEnd.z < 0)
        {
            result.hit = false;
            return result;
        }

        // Convert ray to screen coordinates (also without FOV correction)
        Vector3 rayToScreen = ray.direction;
        Vector3 rayRelative = rayToScreen - Vector3::dot(rayToScreen, forward) * forward;
        float rayX = Vector3::dot(rayRelative, right);
        float rayY = Vector3::dot(rayRelative, up);
        float rayZ = Vector3::dot(rayToScreen, forward);
        float rayScreenX = rayX / rayZ;
        float rayScreenY = rayY / rayZ;

        // Point-to-line distance in 2D screen space
        Vector3 edgeDir2D = Vector3(screenEnd.x - screenStart.x, screenEnd.y - screenStart.y, 0);
        float edgeLength2D = edgeDir2D.length();

        if (edgeLength2D < 1e-6f)
        {
            // Degenerate edge - treat as point
            float distToStart = Vector3(rayScreenX - screenStart.x, rayScreenY - screenStart.y, 0).length();
            if (distToStart <= threshold)
            {
                result.hit = true;
                result.distance = screenStart.z;
                result.point = edgeStart;
                result.edgeParameter = 0.0f;
            }
            else
            {
                result.hit = false;
            }
            result.edgeIndex = edgeIndex;
            return result;
        }

        edgeDir2D = edgeDir2D / edgeLength2D; // Normalize

        // Calculate closest point on edge to ray in 2D
        Vector3 startToRay = Vector3(rayScreenX - screenStart.x, rayScreenY - screenStart.y, 0);
        float t = Vector3::dot(startToRay, edgeDir2D);
        t = std::clamp(t / edgeLength2D, 0.0f, 1.0f); // Normalize to [0,1]

        Vector3 closestPoint2D = Vector3(
            screenStart.x + t * (screenEnd.x - screenStart.x),
            screenStart.y + t * (screenEnd.y - screenStart.y),
            0);

        // Distance in 2D screen space
        float screenDistance = Vector3(rayScreenX - closestPoint2D.x, rayScreenY - closestPoint2D.y, 0).length();

        // Set result values
        result.edgeIndex = edgeIndex;
        result.edgeParameter = t;

        // Interpolate 3D position and calculate ray parameter for proper depth sorting
        Vector3 worldPoint = edgeStart + (edgeEnd - edgeStart) * t;
        result.point = worldPoint;

        // Calculate ray parameter t (distance along ray from origin) to match face rendering
        Vector3 toPoint = worldPoint - ray.origin;
        float rayT = Vector3::dot(toPoint, ray.direction);
        result.distance = std::max(rayT, 0.0f); // Ensure non-negative

        if (screenDistance <= threshold)
        {
            result.hit = true;
            // Distance correction disabled to prevent edges floating above faces
        }
        else
        {
            result.hit = false;
        }

        return result;
    }

    VertexHit intersectVertexScreenSpace(const Ray &ray, const Vector3 &vertex, float threshold, int vertexIndex,
                                         const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUp,
                                         float fov, float aspectRatio)
    {
        VertexHit result;

        // Calculate camera coordinate system
        Vector3 forward = (cameraTarget - cameraPos).normalized();
        Vector3 right = Vector3::cross(forward, cameraUp).normalized();
        Vector3 up = Vector3::cross(right, forward);

        // Project vertex to screen space
        Vector3 toVertex = vertex - cameraPos;
        float z = Vector3::dot(toVertex, forward);
        if (z <= 0.001f)
        {
            result.hit = false; // Behind camera
            return result;
        }

        float x = Vector3::dot(toVertex, right);
        float y = Vector3::dot(toVertex, up);

        // Direct projection onto screen plane at z=1 (no FOV correction)
        float screenX = x / z;
        float screenY = y / z;

        // Convert ray to screen coordinates (also without FOV correction)
        Vector3 rayToScreen = ray.direction;
        Vector3 rayRelative = rayToScreen - Vector3::dot(rayToScreen, forward) * forward;
        float rayX = Vector3::dot(rayRelative, right);
        float rayY = Vector3::dot(rayRelative, up);
        float rayZ = Vector3::dot(rayToScreen, forward);
        float rayScreenX = rayX / rayZ;
        float rayScreenY = rayY / rayZ;

        // Distance in 2D screen space
        float screenDistance = Vector3(rayScreenX - screenX, rayScreenY - screenY, 0).length();

        // Set result values
        result.vertexIndex = vertexIndex;
        result.point = vertex;

        // Calculate ray parameter t (distance along ray from origin) to match face rendering
        Vector3 toPoint = vertex - ray.origin;
        float rayT = Vector3::dot(toPoint, ray.direction);
        result.distance = std::max(rayT, 0.0f); // Ensure non-negative

        if (screenDistance <= threshold)
        {
            result.hit = true;
        }
        else
        {
            result.hit = false;
        }

        return result;
    }

    LineHit intersectLineScreenSpace(const Ray &ray, const Line &line, float threshold, int lineIndex,
                                     const Vector3 &cameraPos, const Vector3 &cameraTarget, const Vector3 &cameraUp,
                                     float fov, float aspectRatio)
    {
        LineHit result;

        // Calculate camera coordinate system
        Vector3 forward = (cameraTarget - cameraPos).normalized();
        Vector3 right = Vector3::cross(forward, cameraUp).normalized();
        Vector3 up = Vector3::cross(right, forward);

        // Project line endpoints to screen space
        auto projectToScreen = [&](const Vector3 &point) -> Vector3
        {
            Vector3 toPoint = point - cameraPos;
            float z = Vector3::dot(toPoint, forward);
            if (z <= 0.001f)
                return Vector3(0, 0, -1); // Behind camera

            float x = Vector3::dot(toPoint, right);
            float y = Vector3::dot(toPoint, up);

            // Direct projection onto screen plane at z=1 (no FOV correction)
            float screenX = x / z;
            float screenY = y / z;

            return Vector3(screenX, screenY, z);
        };

        Vector3 screenStart = projectToScreen(line.start);
        Vector3 screenEnd = projectToScreen(line.end);

        // Check if both points are behind camera
        if (screenStart.z < 0 && screenEnd.z < 0)
        {
            result.hit = false;
            return result;
        }

        // Convert ray to screen coordinates (also without FOV correction)
        Vector3 rayToScreen = ray.direction;
        Vector3 rayRelative = rayToScreen - Vector3::dot(rayToScreen, forward) * forward;
        float rayX = Vector3::dot(rayRelative, right);
        float rayY = Vector3::dot(rayRelative, up);
        float rayZ = Vector3::dot(rayToScreen, forward);
        float rayScreenX = rayX / rayZ;
        float rayScreenY = rayY / rayZ;

        // Point-to-line distance in 2D screen space
        Vector3 lineDir2D = Vector3(screenEnd.x - screenStart.x, screenEnd.y - screenStart.y, 0);
        float lineLength2D = lineDir2D.length();

        if (lineLength2D < 1e-6f)
        {
            // Degenerate line - treat as point
            float distToStart = Vector3(rayScreenX - screenStart.x, rayScreenY - screenStart.y, 0).length();
            if (distToStart <= threshold * line.thickness)
            {
                result.hit = true;
                // Calculate ray parameter for proper depth sorting
                Vector3 toPoint = line.start - ray.origin;
                float rayT = Vector3::dot(toPoint, ray.direction);
                result.distance = std::max(rayT, 0.0f);
                result.point = line.start;
                result.lineParameter = 0.0f;
            }
            else
            {
                result.hit = false;
            }
            result.lineIndex = lineIndex;
            return result;
        }

        lineDir2D = lineDir2D / lineLength2D; // Normalize

        // Calculate closest point on line to ray in 2D
        Vector3 startToRay = Vector3(rayScreenX - screenStart.x, rayScreenY - screenStart.y, 0);
        float t = Vector3::dot(startToRay, lineDir2D);
        t = std::clamp(t / lineLength2D, 0.0f, 1.0f); // Normalize to [0,1]

        Vector3 closestPoint2D = Vector3(
            screenStart.x + t * (screenEnd.x - screenStart.x),
            screenStart.y + t * (screenEnd.y - screenStart.y),
            0);

        // Distance in 2D screen space
        float screenDistance = Vector3(rayScreenX - closestPoint2D.x, rayScreenY - closestPoint2D.y, 0).length();

        // Set result values
        result.lineIndex = lineIndex;
        result.lineParameter = t;

        // Interpolate 3D position and calculate ray parameter for proper depth sorting
        Vector3 worldPoint = line.start + (line.end - line.start) * t;
        result.point = worldPoint;

        // Calculate ray parameter t (distance along ray from origin) to match face rendering
        Vector3 toPoint = worldPoint - ray.origin;
        float rayT = Vector3::dot(toPoint, ray.direction);
        result.distance = std::max(rayT, 0.0f); // Ensure non-negative

        if (screenDistance <= threshold * line.thickness)
        {
            result.hit = true;
            // Distance correction disabled to prevent lines floating above faces
        }
        else
        {
            result.hit = false;
        }

        return result;
    }

    LineHit intersectLine(const Ray &ray, const Line &line, float threshold, int lineIndex)
    {
        LineHit result;

        float rayParam, lineParam;
        float distance = rayEdgeDistance(ray, line.start, line.end, rayParam, lineParam);

        // Use line thickness as additional threshold factor
        float effectiveThreshold = threshold * line.thickness;

        if (distance <= effectiveThreshold)
        {
            result.hit = true;
            result.distance = rayParam;
            result.point = line.start + (line.end - line.start) * lineParam;
            result.lineIndex = lineIndex;
            result.lineParameter = lineParam;
        }

        return result;
    }

    bool isVertexVisible(const Vector3 &cameraPos, const Vector3 &vertex, const class Model &model)
    {
        // Create ray from camera to vertex
        Vector3 direction = (vertex - cameraPos).normalized();
        float targetDistance = (vertex - cameraPos).length();
        Ray visibilityRay(cameraPos, direction);

        // Check if any face occludes this vertex
        const auto &faces = model.getFaces();
        const auto &vertices = model.getVertices();

        for (size_t i = 0; i < faces.size(); ++i)
        {
            const auto &face = faces[i];

            // Skip invalid faces
            if (face.v1 >= vertices.size() || face.v2 >= vertices.size() || face.v3 >= vertices.size())
            {
                continue;
            }

            const Vector3 &v0 = vertices[face.v1].position;
            const Vector3 &v1 = vertices[face.v2].position;
            const Vector3 &v2 = vertices[face.v3].position;

            // Skip faces that contain the target vertex (allow selection of vertices on face edges)
            bool containsTargetVertex = false;
            if ((v0 - vertex).length() < 0.001f || (v1 - vertex).length() < 0.001f || (v2 - vertex).length() < 0.001f)
            {
                containsTargetVertex = true;
            }
            if (containsTargetVertex)
                continue;

            TriangleHit hit = intersectTriangle(visibilityRay, v0, v1, v2);

            // If ray hits a face closer than the target vertex, it's occluded
            // Use larger tolerance to allow selection of vertices near face edges
            if (hit.hit && hit.distance < (targetDistance - 0.05f))
            {
                return false;
            }
        }

        return true; // No occlusion found
    }

    RaycastResult findClosestIntersection(const Ray &ray, const class Model &model, float vertexThreshold, float edgeThreshold)
    {
        RaycastResult result;
        float closestDistance = std::numeric_limits<float>::max();

        const auto &vertices = model.getVertices();
        const auto &edges = model.getEdges();
        const auto &faces = model.getFaces();

        // Check vertex intersections
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            VertexHit vertexHit = intersectVertex(ray, vertices[i].position, vertexThreshold, static_cast<int>(i));

            if (vertexHit.hit && vertexHit.distance < closestDistance)
            {
                closestDistance = vertexHit.distance;
                result.type = RaycastResultType::VERTEX;
                result.distance = vertexHit.distance;
                result.point = vertexHit.point;
                result.elementIndex = vertexHit.vertexIndex;
                result.vertexHit = vertexHit;
            }
        }

        // Check edge intersections
        for (size_t i = 0; i < edges.size(); ++i)
        {
            const auto &edge = edges[i];

            // Skip invalid edges
            if (edge.v1 >= vertices.size() || edge.v2 >= vertices.size())
            {
                continue;
            }

            const Vector3 &edgeStart = vertices[edge.v1].position;
            const Vector3 &edgeEnd = vertices[edge.v2].position;

            EdgeHit edgeHit = intersectEdge(ray, edgeStart, edgeEnd, edgeThreshold, static_cast<int>(i));

            if (edgeHit.hit && edgeHit.distance < closestDistance)
            {
                closestDistance = edgeHit.distance;
                result.type = RaycastResultType::EDGE;
                result.distance = edgeHit.distance;
                result.point = edgeHit.point;
                result.elementIndex = edgeHit.edgeIndex;
                result.edgeHit = edgeHit;
            }
        }

        // Check face intersections
        for (size_t i = 0; i < faces.size(); ++i)
        {
            const auto &face = faces[i];

            // Skip invalid faces
            if (face.v1 >= vertices.size() || face.v2 >= vertices.size() || face.v3 >= vertices.size())
            {
                continue;
            }

            const Vector3 &v0 = vertices[face.v1].position;
            const Vector3 &v1 = vertices[face.v2].position;
            const Vector3 &v2 = vertices[face.v3].position;

            TriangleHit triangleHit = intersectTriangle(ray, v0, v1, v2);

            if (triangleHit.hit && triangleHit.distance < closestDistance)
            {
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