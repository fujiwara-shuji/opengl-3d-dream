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

struct RenderConfig {
    // Visual display thresholds (for rendering appearance)
    float vertexDisplayRadius = 0.02f;   // Visual radius of vertices when rendered
    float edgeDisplayThickness = 0.01f;  // Visual thickness of edges when rendered
    float lineThickness = 0.01f;        // Thickness multiplier for coordinate axes

    // Selection thresholds (for mouse interaction)
    float vertexSelectionThreshold = 0.05f;  // Click range for vertex selection
    float edgeSelectionThreshold = 0.02f;    // Click range for edge selection

    // Distance epsilon for avoiding self-intersection
    float rayEpsilon = 0.001f;

    // Display settings
    bool showVertices = true;          // Show/hide vertices
    bool showEdges = true;             // Show/hide model edges
    bool showFaces = true;             // Show/hide faces
    bool showCoordinateAxes = true;    // Show/hide coordinate axes

    // Default constructor
    RenderConfig() = default;
};

// Reflection settings structure
struct ReflectionConfig {
    // Reflection control
    bool enableReflection = false;           // Enable/disable specular reflection system
    int maxReflectionDepth = 5;              // Maximum recursion depth for reflections
    float reflectionEpsilon = 0.001f;        // Offset distance to avoid self-intersection

    // Lambert diffuse reflection control
    bool enableLambertDiffuse = true;        // Enable/disable Lambert diffuse reflection
    Vector3 lightDirection = Vector3(1, -1, 1).normalized();  // Light source direction
    float ambientStrength = 0.2f;            // Ambient light strength (0.0 - 1.0)
    float diffuseStrength = 0.8f;            // Diffuse light strength (0.0 - 1.0)

    // Surface reflection properties
    float frontFaceReflectionAlpha = 0.3f;   // Front face reflection strength (0.0 = no reflection, 1.0 = full reflection)
    float backFaceReflectionAlpha = 0.1f;    // Back face reflection strength

    // Surface colors (when not reflecting)
    Vector3 frontFaceColor = Vector3(0.6f, 0.6f, 0.6f);  // Front face base color
    Vector3 backFaceColor = Vector3(0.4f, 0.4f, 0.4f);   // Back face base color

    // Default constructor
    ReflectionConfig() = default;
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

    // Render configuration
    RenderConfig config;
    ReflectionConfig reflectionConfig;

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

    // Configuration access
    RenderConfig& getRenderConfig() { return config; }
    const RenderConfig& getRenderConfig() const { return config; }

    ReflectionConfig& getReflectionConfig() { return reflectionConfig; }
    const ReflectionConfig& getReflectionConfig() const { return reflectionConfig; }

    // Visual display settings
    void setVertexDisplayRadius(float radius) { config.vertexDisplayRadius = radius; }
    void setEdgeDisplayThickness(float thickness) { config.edgeDisplayThickness = thickness; }
    void setLineThickness(float thickness) { config.lineThickness = thickness; }

    // Selection settings
    void setVertexSelectionThreshold(float threshold) { config.vertexSelectionThreshold = threshold; }
    void setEdgeSelectionThreshold(float threshold) { config.edgeSelectionThreshold = threshold; }

    // Getters
    float getVertexDisplayRadius() const { return config.vertexDisplayRadius; }
    float getEdgeDisplayThickness() const { return config.edgeDisplayThickness; }
    float getVertexSelectionThreshold() const { return config.vertexSelectionThreshold; }
    float getEdgeSelectionThreshold() const { return config.edgeSelectionThreshold; }

    // Display settings control
    void setShowVertices(bool show) { config.showVertices = show; }
    void setShowEdges(bool show) { config.showEdges = show; }
    void setShowFaces(bool show) { config.showFaces = show; }
    void setShowCoordinateAxes(bool show) { config.showCoordinateAxes = show; }
    bool getShowVertices() const { return config.showVertices; }
    bool getShowEdges() const { return config.showEdges; }
    bool getShowFaces() const { return config.showFaces; }
    bool getShowCoordinateAxes() const { return config.showCoordinateAxes; }

    // Debug
    void saveAsText(const std::string& filename) const;

private:
    // Internal rendering methods
    Ray generateCameraRay(int x, int y) const;
    Vector3 castRay(const Ray& ray, int depth = 0) const;
    Vector3 calculateSkyboxColor(const Ray& ray) const;
};