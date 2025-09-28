#pragma once

#include "IRenderer.h"
#include "../math/Vector3.h"
#include "../core/Ray.h"
#include "../core/Model.h"
#include "../core/Camera.h"
#include <vector>
#include <memory>

struct Triangle {
    Vector3 v0, v1, v2;
    Vector3 color;

    Triangle() = default;
    Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& color = Vector3(0.5f, 0.5f, 0.5f))
        : v0(v0), v1(v1), v2(v2), color(color) {}
};

class SoftwareRenderer : public IRenderer {
private:
    int width = 640;
    int height = 480;
    std::vector<Vector3> pixels;
    std::vector<Triangle> triangles;
    std::vector<Line> lines;  // Coordinate axes and other lines
    std::vector<Vector3> vertices;  // Vertices to render as points
    std::vector<Line> edges;  // Model edges to render as lines

    // Camera parameters
    Vector3 cameraPos = Vector3(0, 0, 5);
    Vector3 cameraTarget = Vector3(0, 0, 0);
    Vector3 cameraUp = Vector3(0, 0, 1);
    float fov = 45.0f * 3.14159f / 180.0f; // 45 degrees in radians
    float aspectRatio = 4.0f / 3.0f;

public:
    SoftwareRenderer() = default;
    ~SoftwareRenderer() = default;

    // IRenderer interface
    void initialize() override;
    void shutdown() override;
    void setResolution(int width, int height) override;
    void render() override;
    void render(const Model& model, const Camera& camera) override;
    const std::vector<Vector3>& getPixelData() const override;
    void clear(const Vector3& clearColor) override;

    // Scene management
    void addTriangle(const Triangle& triangle);
    void clearTriangles();

    // Line management for coordinate axes
    void addLine(const Line& line);
    void clearLines();
    void setLines(const std::vector<Line>& lineList);

    // Vertex and edge rendering for models
    void addVertex(const Vector3& vertex);
    void clearVertices();
    void setVertices(const std::vector<Vector3>& vertexList);
    void addEdge(const Line& edge);
    void clearEdges();
    void setEdges(const std::vector<Line>& edgeList);

    // Camera control
    void setCamera(const Vector3& pos, const Vector3& target, const Vector3& up);
    void setCameraFOV(float fovDegrees);

    // Debug
    void saveAsText(const std::string& filename) const;

private:
    // Internal rendering methods
    Ray generateCameraRay(int x, int y) const;
    Vector3 castRay(const Ray& ray) const;
    Vector3 calculateSkyboxColor(const Ray& ray) const;
};