#include "SoftwareRenderer.h"
#include "../utils/Utils.h"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <string>

void SoftwareRenderer::initialize()
{
    Utils::logInfo("Initializing Software Renderer");
    setResolution(width, height);
    Utils::logInfo("Software Renderer initialized with resolution " +
                   std::to_string(width) + "x" + std::to_string(height));
}

void SoftwareRenderer::shutdown()
{
    Utils::logInfo("Shutting down Software Renderer");
    pixels.clear();
    triangles.clear();
}

void SoftwareRenderer::setResolution(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;
    aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // Resize pixel buffer
    pixels.resize(width * height);
    clear(Vector3(0.1f, 0.1f, 0.2f)); // Default dark blue background

    Utils::logInfo("Resolution set to " + std::to_string(width) + "x" + std::to_string(height));
}

void SoftwareRenderer::clear(const Vector3 &clearColor)
{
    std::fill(pixels.begin(), pixels.end(), clearColor);
}

void SoftwareRenderer::render()
{
    // Clear screen with sky gradient
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Ray ray = generateCameraRay(x, y);
            Vector3 color = castRay(ray);

            // Clamp color values to [0, 1] range
            color.x = Utils::clamp(color.x, 0.0f, 1.0f);
            color.y = Utils::clamp(color.y, 0.0f, 1.0f);
            color.z = Utils::clamp(color.z, 0.0f, 1.0f);

            pixels[y * width + x] = color;
        }
    }
}

void SoftwareRenderer::render(const Model &model, const Camera &camera)
{
    // Set camera parameters from Camera object
    cameraPos = camera.getPosition();
    cameraTarget = camera.getTarget();
    cameraUp = Vector3(0, 0, 1); // Z-up coordinate system
    fov = camera.getFOV();

    // Clear existing triangles and convert Model to triangles
    clearTriangles();

    const auto &vertices = model.getVertices();
    const auto &faces = model.getFaces();

    // Convert Model faces to renderer triangles
    for (const auto &face : faces)
    {
        if (face.v1 < vertices.size() && face.v2 < vertices.size() && face.v3 < vertices.size())
        {
            const Vector3 &v0 = vertices[face.v1].position;
            const Vector3 &v1 = vertices[face.v2].position;
            const Vector3 &v2 = vertices[face.v3].position;

            // Use a default color for now (will be enhanced later)
            Vector3 color(0.7f, 0.7f, 0.7f); // Light gray

            triangles.emplace_back(v0, v1, v2, color);
        }
    }

    // Render the scene
    render();
}

const std::vector<Vector3> &SoftwareRenderer::getPixelData() const
{
    return pixels;
}

void SoftwareRenderer::addTriangle(const Triangle &triangle)
{
    triangles.push_back(triangle);
    Utils::logInfo("Added triangle to scene (total: " + std::to_string(triangles.size()) + ")");
}

void SoftwareRenderer::clearTriangles()
{
    triangles.clear();
    Utils::logInfo("Cleared all triangles from scene");
}

void SoftwareRenderer::addLine(const Line &line)
{
    lines.push_back(line);
    Utils::logInfo("Added line to scene (total: " + std::to_string(lines.size()) + ")");
}

void SoftwareRenderer::clearLines()
{
    lines.clear();
    Utils::logInfo("Cleared all lines from scene");
}

void SoftwareRenderer::setLines(const std::vector<Line> &lineList)
{
    lines = lineList;
    Utils::logInfo("Set " + std::to_string(lines.size()) + " lines in scene");
}

void SoftwareRenderer::addVertex(const Vector3 &vertex)
{
    vertices.push_back(vertex);
    Utils::logInfo("Added vertex to scene (total: " + std::to_string(vertices.size()) + ")");
}

void SoftwareRenderer::clearVertices()
{
    vertices.clear();
    Utils::logInfo("Cleared all vertices from scene");
}

void SoftwareRenderer::setVertices(const std::vector<Vector3> &vertexList)
{
    vertices = vertexList;
    Utils::logInfo("Set " + std::to_string(vertices.size()) + " vertices in scene");
}

void SoftwareRenderer::addEdge(const Line &edge)
{
    edges.push_back(edge);
    Utils::logInfo("Added edge to scene (total: " + std::to_string(edges.size()) + ")");
}

void SoftwareRenderer::clearEdges()
{
    edges.clear();
    Utils::logInfo("Cleared all edges from scene");
}

void SoftwareRenderer::setEdges(const std::vector<Line> &edgeList)
{
    edges = edgeList;
    Utils::logInfo("Set " + std::to_string(edges.size()) + " edges in scene");
}

void SoftwareRenderer::setCamera(const Vector3 &pos, const Vector3 &target, const Vector3 &up)
{
    cameraPos = pos;
    cameraTarget = target;
    cameraUp = up.normalized();
    Utils::logInfo("Camera set: pos=" + std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z));
}

void SoftwareRenderer::setCameraFOV(float fovDegrees)
{
    fov = fovDegrees * Utils::DEG_TO_RAD;
    Utils::logInfo("Camera FOV set to " + std::to_string(fovDegrees) + " degrees");
}

Ray SoftwareRenderer::generateCameraRay(int x, int y) const
{
    // Convert screen coordinates to normalized device coordinates [-1, 1]
    float normalizedX = (2.0f * x / width) - 1.0f;
    float normalizedY = 1.0f - (2.0f * y / height); // Standard screen to NDC conversion

    // Calculate camera coordinate system (right-hand rule)
    Vector3 forward = (cameraTarget - cameraPos).normalized();
    Vector3 right = Vector3::cross(forward, cameraUp).normalized();
    Vector3 up = cameraUp.normalized(); // Use provided up vector directly

    // Calculate ray direction
    float tanHalfFov = std::tan(fov * 0.5f);
    Vector3 rayDir = forward +
                     right * (normalizedX * aspectRatio * tanHalfFov) +
                     up * (normalizedY * tanHalfFov);

    return Ray(cameraPos, rayDir.normalized());
}

Vector3 SoftwareRenderer::castRay(const Ray &ray) const
{
    float closestDistance = std::numeric_limits<float>::max();
    bool hitFound = false;
    Vector3 hitColor;

    // Test ray against vertices first (highest priority) - only if vertices are enabled
    if (config.showVertices)
    {
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            VertexHit vertexHit = RayIntersection::intersectVertexScreenSpace(ray, vertices[i], config.vertexDisplayRadius, static_cast<int>(i),
                                                                              cameraPos, cameraTarget, cameraUp, fov, aspectRatio);

            if (vertexHit.hit && vertexHit.distance < closestDistance && vertexHit.distance > config.rayEpsilon)
            {
                closestDistance = vertexHit.distance;
                hitColor = Vector3(1.0f, 1.0f, 1.0f); // White for vertices
                hitFound = true;
            }
        }
    }

    // Test ray against edges - only if edges are enabled
    if (config.showEdges)
    {
        for (size_t i = 0; i < edges.size(); ++i)
        {
            const Line &edge = edges[i];
            EdgeHit edgeHit = RayIntersection::intersectEdgeScreenSpace(ray, edge.start, edge.end, config.edgeDisplayThickness, static_cast<int>(i),
                                                                        cameraPos, cameraTarget, cameraUp, fov, aspectRatio);

            if (edgeHit.hit && edgeHit.distance < closestDistance && edgeHit.distance > config.rayEpsilon)
            {
                closestDistance = edgeHit.distance;
                hitColor = Vector3(0.7f, 0.7f, 0.7f); // Light gray for edges
                hitFound = true;
            }
        }
    }

    // Test ray against coordinate axes (should be visible over triangles when very close) - only if axes are enabled
    if (config.showCoordinateAxes)
    {
        for (size_t i = 0; i < lines.size(); ++i)
        {
            const Line &line = lines[i];
            LineHit lineHit = RayIntersection::intersectLineScreenSpace(ray, line, config.lineThickness, static_cast<int>(i),
                                                                        cameraPos, cameraTarget, cameraUp, fov, aspectRatio);

            if (lineHit.hit && lineHit.distance < closestDistance && lineHit.distance > config.rayEpsilon)
            {
                closestDistance = lineHit.distance;
                hitColor = line.color;
                hitFound = true;
            }
        }
    }

    // Test ray against all triangles - only if faces are enabled
    if (config.showFaces)
    {
        TriangleHit closestTriangleHit;
        for (const Triangle &triangle : triangles)
        {
            TriangleHit hit = RayIntersection::intersectTriangle(ray, triangle.v0, triangle.v1, triangle.v2);

            if (hit.hit && hit.distance < closestDistance && hit.distance > config.rayEpsilon)
            {
                closestTriangleHit = hit;
                closestDistance = hit.distance;
                hitFound = true;

                // Find triangle color
                float shading = hit.isFrontFace ? 1.0f : 0.7f;
                hitColor = triangle.color * shading;
            }
        }
    }

    if (hitFound)
    {
        return hitColor;
    }

    // No hit - return sky color
    return calculateSkyboxColor(ray);
}

Vector3 SoftwareRenderer::calculateSkyboxColor(const Ray &ray) const
{
    // Sky gradient based on ray direction (from CLAUDE.md)
    Vector3 skyLightDirection = Vector3(1, -1, 1).normalized(); // Light source direction
    Vector3 skyDarkColor = Vector3(0.1f, 0.1f, 0.2f);           // Dark blue
    Vector3 skyBrightColor = Vector3(0.8f, 0.9f, 1.0f);         // Light blue

    // Calculate alignment with light direction
    float alignment = Vector3::dot(ray.direction, skyLightDirection);

    // Map from [-1, 1] to [0, 1]
    float t = (alignment + 1.0f) * 0.5f;

    // Linear interpolation between dark and bright colors
    return Vector3::lerp(skyDarkColor, skyBrightColor, t);
}

void SoftwareRenderer::saveAsText(const std::string &filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        Utils::logError("Cannot create file: " + filename);
        return;
    }

    file << "Software Renderer Output (" << width << "x" << height << ")\n";
    file << "Triangles: " << triangles.size() << "\n";
    file << "Camera: pos=" << cameraPos << " target=" << cameraTarget << "\n\n";

    // Sample a few pixels for verification
    const int sampleStep = std::max(1, width / 20);
    for (int y = 0; y < height; y += sampleStep)
    {
        for (int x = 0; x < width; x += sampleStep)
        {
            Vector3 color = pixels[y * width + x];
            file << "(" << std::setw(3) << x << "," << std::setw(3) << y << "): "
                 << std::fixed << std::setprecision(2)
                 << "R=" << color.x << " G=" << color.y << " B=" << color.z << "\n";
        }
        file << "\n";
    }

    file.close();
    Utils::logInfo("Renderer output saved to " + filename);
}