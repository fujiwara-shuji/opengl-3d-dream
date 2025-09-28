#include "Model.h"
#include "Ray.h"
#include "Camera.h"
#include "../utils/Utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>

Model::Model() : isModified(false), selectedVertexIndex(-1) {
}

bool Model::loadFromFile(const std::string& filePath) {
    if (!Utils::fileExists(filePath)) {
        Utils::logError("File not found: " + filePath);
        return false;
    }

    // Check file extension
    if (filePath.substr(filePath.find_last_of(".") + 1) != "fjwr") {
        Utils::logError("Unsupported file format. Expected .fjwr");
        return false;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        Utils::logError("Cannot open file: " + filePath);
        return false;
    }

    // Read entire file content
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    if (!parseFjwrFile(content)) {
        Utils::logError("Failed to parse .fjwr file: " + filePath);
        return false;
    }

    filename = filePath;
    isModified = false;

    // Calculate normals and generate edges
    calculateNormals();
    generateEdgesFromFaces();

    Utils::logInfo("Loaded model from " + filePath +
                   " (vertices: " + std::to_string(vertices.size()) +
                   ", faces: " + std::to_string(faces.size()) +
                   ", edges: " + std::to_string(edges.size()) + ")");

    return true;
}

bool Model::saveToFile(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        Utils::logError("Cannot create file: " + filePath);
        return false;
    }

    std::string content = generateFjwrContent();
    file << content;
    file.close();

    filename = filePath;
    isModified = false;

    Utils::logInfo("Saved model to " + filePath);
    return true;
}

bool Model::saveAs(const std::string& filePath) {
    return saveToFile(filePath);
}

void Model::addVertex(const Vertex& vertex) {
    vertices.push_back(vertex);
    markAsModified();
}

void Model::addVertex(const Vector3& position) {
    vertices.emplace_back(position);
    markAsModified();
}

void Model::addVertex(float x, float y, float z) {
    vertices.emplace_back(x, y, z);
    markAsModified();
}

void Model::addFace(const Face& face) {
    if (isFaceValid(face)) {
        faces.push_back(face);
        markAsModified();
    } else {
        Utils::logError("Invalid face indices: " + std::to_string(face.v1) +
                       ", " + std::to_string(face.v2) +
                       ", " + std::to_string(face.v3));
    }
}

void Model::addFace(int v1, int v2, int v3) {
    addFace(Face(v1, v2, v3));
}

void Model::addEdge(const Edge& edge) {
    if (isEdgeValid(edge)) {
        edges.push_back(edge);
        markAsModified();
    } else {
        Utils::logError("Invalid edge indices: " + std::to_string(edge.v1) +
                       ", " + std::to_string(edge.v2));
    }
}

void Model::addEdge(int v1, int v2) {
    addEdge(Edge(v1, v2));
}

void Model::removeVertex(int index) {
    if (!isVertexIndexValid(index)) {
        Utils::logError("Invalid vertex index: " + std::to_string(index));
        return;
    }

    vertices.erase(vertices.begin() + index);

    // Remove faces and edges that reference this vertex
    faces.erase(std::remove_if(faces.begin(), faces.end(),
        [index](const Face& face) {
            return face.v1 == index || face.v2 == index || face.v3 == index;
        }), faces.end());

    edges.erase(std::remove_if(edges.begin(), edges.end(),
        [index](const Edge& edge) {
            return edge.v1 == index || edge.v2 == index;
        }), edges.end());

    // Update indices in remaining faces and edges
    for (auto& face : faces) {
        if (face.v1 > index) face.v1--;
        if (face.v2 > index) face.v2--;
        if (face.v3 > index) face.v3--;
    }

    for (auto& edge : edges) {
        if (edge.v1 > index) edge.v1--;
        if (edge.v2 > index) edge.v2--;
    }

    markAsModified();
}

void Model::removeFace(int index) {
    if (index >= 0 && index < static_cast<int>(faces.size())) {
        faces.erase(faces.begin() + index);
        markAsModified();
    } else {
        Utils::logError("Invalid face index: " + std::to_string(index));
    }
}

void Model::removeEdge(int index) {
    if (index >= 0 && index < static_cast<int>(edges.size())) {
        edges.erase(edges.begin() + index);
        markAsModified();
    } else {
        Utils::logError("Invalid edge index: " + std::to_string(index));
    }
}

void Model::setVertexPosition(int index, const Vector3& position) {
    if (isVertexIndexValid(index)) {
        vertices[index].position = position;
        calculateNormals();  // Recalculate normals when vertex moves
        markAsModified();
    } else {
        Utils::logError("Invalid vertex index: " + std::to_string(index));
    }
}

Vector3 Model::getVertexPosition(int index) const {
    if (isVertexIndexValid(index)) {
        return vertices[index].position;
    } else {
        Utils::logError("Invalid vertex index: " + std::to_string(index));
        return Vector3(0, 0, 0);
    }
}

void Model::clear() {
    vertices.clear();
    faces.clear();
    edges.clear();
    filename.clear();
    isModified = false;
}

void Model::calculateNormals() {
    // Reset all normals to zero
    for (auto& vertex : vertices) {
        vertex.normal = Vector3(0, 0, 0);
    }

    // Calculate face normals and accumulate to vertices
    for (const auto& face : faces) {
        if (!isFaceValid(face)) continue;

        const Vector3& v0 = vertices[face.v1].position;
        const Vector3& v1 = vertices[face.v2].position;
        const Vector3& v2 = vertices[face.v3].position;

        // Calculate face normal (CCW = front face)
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 faceNormal = Vector3::cross(edge1, edge2).normalized();

        // Accumulate to vertex normals
        vertices[face.v1].normal = vertices[face.v1].normal + faceNormal;
        vertices[face.v2].normal = vertices[face.v2].normal + faceNormal;
        vertices[face.v3].normal = vertices[face.v3].normal + faceNormal;
    }

    // Normalize accumulated normals
    for (auto& vertex : vertices) {
        float length = vertex.normal.length();
        if (length > 0.001f) {
            vertex.normal = vertex.normal / length;
        } else {
            vertex.normal = Vector3(0, 0, 1);  // Default up normal
        }
    }
}

void Model::createCube(float size) {
    clear();

    float half = size * 0.5f;

    // Add 8 vertices of a cube
    addVertex(-half, -half, -half);  // 0
    addVertex( half, -half, -half);  // 1
    addVertex( half,  half, -half);  // 2
    addVertex(-half,  half, -half);  // 3
    addVertex(-half, -half,  half);  // 4
    addVertex( half, -half,  half);  // 5
    addVertex( half,  half,  half);  // 6
    addVertex(-half,  half,  half);  // 7

    // Add 12 faces (2 triangles per cube face, CCW winding)
    // Bottom face (z = -half)
    addFace(0, 2, 1);
    addFace(0, 3, 2);

    // Top face (z = half)
    addFace(4, 5, 6);
    addFace(4, 6, 7);

    // Front face (y = -half)
    addFace(0, 1, 5);
    addFace(0, 5, 4);

    // Back face (y = half)
    addFace(2, 7, 6);
    addFace(2, 3, 7);

    // Left face (x = -half)
    addFace(0, 4, 7);
    addFace(0, 7, 3);

    // Right face (x = half)
    addFace(1, 6, 5);
    addFace(1, 2, 6);

    calculateNormals();
    generateEdgesFromFaces();

    Utils::logInfo("Created cube with size " + std::to_string(size));
}

void Model::createTriangle() {
    clear();

    // Create a simple triangle in XY plane
    addVertex(0.0f, 0.0f, 0.0f);    // 0
    addVertex(1.0f, 0.0f, 0.0f);    // 1
    addVertex(0.5f, 1.0f, 0.0f);    // 2

    addFace(0, 1, 2);  // CCW winding

    calculateNormals();
    generateEdgesFromFaces();

    Utils::logInfo("Created triangle");
}

void Model::createQuad() {
    clear();

    // Create a quad in XY plane
    addVertex(0.0f, 0.0f, 0.0f);    // 0
    addVertex(1.0f, 0.0f, 0.0f);    // 1
    addVertex(1.0f, 1.0f, 0.0f);    // 2
    addVertex(0.0f, 1.0f, 0.0f);    // 3

    // Two triangles with CCW winding
    addFace(0, 1, 2);
    addFace(0, 2, 3);

    calculateNormals();
    generateEdgesFromFaces();

    Utils::logInfo("Created quad");
}

bool Model::isValid() const {
    // Check if all faces reference valid vertices
    for (const auto& face : faces) {
        if (!isFaceValid(face)) {
            return false;
        }
    }

    // Check if all edges reference valid vertices
    for (const auto& edge : edges) {
        if (!isEdgeValid(edge)) {
            return false;
        }
    }

    return true;
}

bool Model::isVertexIndexValid(int index) const {
    return index >= 0 && index < static_cast<int>(vertices.size());
}

bool Model::isFaceValid(const Face& face) const {
    return isVertexIndexValid(face.v1) &&
           isVertexIndexValid(face.v2) &&
           isVertexIndexValid(face.v3) &&
           face.v1 != face.v2 &&
           face.v2 != face.v3 &&
           face.v3 != face.v1;
}

bool Model::isEdgeValid(const Edge& edge) const {
    return isVertexIndexValid(edge.v1) &&
           isVertexIndexValid(edge.v2) &&
           edge.v1 != edge.v2;
}

bool Model::parseFjwrFile(const std::string& content) {
    clear();

    std::istringstream stream(content);
    std::string line;
    int lineNumber = 0;

    while (std::getline(stream, line)) {
        lineNumber++;

        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream lineStream(line);
        std::string type;
        lineStream >> type;

        if (type == "v") {
            // Vertex: v x y z
            float x, y, z;
            if (lineStream >> x >> y >> z) {
                addVertex(x, y, z);
            } else {
                Utils::logError("Invalid vertex format at line " + std::to_string(lineNumber));
                return false;
            }
        }
        else if (type == "f") {
            // Face: f v1 v2 v3
            int v1, v2, v3;
            if (lineStream >> v1 >> v2 >> v3) {
                addFace(v1, v2, v3);
            } else {
                Utils::logError("Invalid face format at line " + std::to_string(lineNumber));
                return false;
            }
        }
        else if (type == "l") {
            // Edge: l v1 v2
            int v1, v2;
            if (lineStream >> v1 >> v2) {
                addEdge(v1, v2);
            } else {
                Utils::logError("Invalid edge format at line " + std::to_string(lineNumber));
                return false;
            }
        }
        else {
            Utils::logError("Unknown line type '" + type + "' at line " + std::to_string(lineNumber));
            return false;
        }
    }

    return true;
}

std::string Model::generateFjwrContent() const {
    std::ostringstream content;

    content << "# 3D Model exported from 3D Model Editor\n";
    content << "# Format: .fjwr (Fujiwara format)\n";
    content << "# v x y z          - vertex\n";
    content << "# f v1 v2 v3       - face (CCW winding)\n";
    content << "# l v1 v2          - edge\n\n";

    // Write vertices
    for (const auto& vertex : vertices) {
        content << "v " << vertex.position.x << " "
                << vertex.position.y << " "
                << vertex.position.z << "\n";
    }

    if (!vertices.empty() && !faces.empty()) {
        content << "\n";
    }

    // Write faces
    for (const auto& face : faces) {
        content << "f " << face.v1 << " " << face.v2 << " " << face.v3 << "\n";
    }

    if (!faces.empty() && !edges.empty()) {
        content << "\n";
    }

    // Write edges
    for (const auto& edge : edges) {
        content << "l " << edge.v1 << " " << edge.v2 << "\n";
    }

    return content.str();
}

void Model::generateEdgesFromFaces() {
    std::set<std::pair<int, int>> edgeSet;

    // Generate edges from all faces
    for (const auto& face : faces) {
        if (!isFaceValid(face)) continue;

        // Add three edges of the triangle
        auto addEdgeToSet = [&](int v1, int v2) {
            if (v1 > v2) std::swap(v1, v2);  // Normalize order
            edgeSet.insert({v1, v2});
        };

        addEdgeToSet(face.v1, face.v2);
        addEdgeToSet(face.v2, face.v3);
        addEdgeToSet(face.v3, face.v1);
    }

    // Clear existing auto-generated edges and add new ones
    // Note: This will remove manually added edges too
    // In the future, we might want to distinguish between auto and manual edges
    edges.clear();

    for (const auto& edgePair : edgeSet) {
        edges.emplace_back(edgePair.first, edgePair.second);
    }
}

// Selection system implementation
void Model::setSelectedVertex(int index) {
    if (isVertexIndexValid(index)) {
        selectedVertexIndex = index;
        Utils::logInfo("Selected vertex " + std::to_string(index) +
                      " at position " + std::to_string(vertices[index].position.x) +
                      "," + std::to_string(vertices[index].position.y) +
                      "," + std::to_string(vertices[index].position.z));
    } else {
        Utils::logError("Invalid vertex index for selection: " + std::to_string(index));
    }
}

void Model::clearSelection() {
    if (selectedVertexIndex >= 0) {
        Utils::logInfo("Cleared selection (was vertex " + std::to_string(selectedVertexIndex) + ")");
        selectedVertexIndex = -1;
    }
}

Vector3 Model::getSelectedVertexPosition() const {
    if (hasSelection()) {
        return vertices[selectedVertexIndex].position;
    }
    return Vector3(0, 0, 0);
}

bool Model::selectVertex(const Ray& ray, const Camera& camera, float baseThreshold) {
    // Calculate dynamic threshold based on camera distance
    float cameraDistance = camera.getDistance();
    float dynamicThreshold = baseThreshold * cameraDistance * 0.1f;

    // Find closest vertex intersection with visibility check
    int closestVertexIndex = -1;
    float closestDistance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < vertices.size(); ++i) {
        const Vector3& vertexPos = vertices[i].position;

        // Check if vertex is visible (not occluded by faces)
        if (!RayIntersection::isVertexVisible(camera.getPosition(), vertexPos, *this)) {
            continue;  // Skip occluded vertices
        }

        // Check intersection with threshold
        VertexHit hit = RayIntersection::intersectVertex(ray, vertexPos, dynamicThreshold, static_cast<int>(i));

        if (hit.hit && hit.distance < closestDistance) {
            closestDistance = hit.distance;
            closestVertexIndex = hit.vertexIndex;
        }
    }

    // Check if we should deselect (clicked far from any vertex)
    if (closestVertexIndex == -1) {
        // Calculate deselection threshold (larger than selection threshold)
        float deselectionThreshold = dynamicThreshold * 2.0f;

        // If we have a selection and clicked far from it, deselect
        if (hasSelection()) {
            const Vector3& selectedPos = vertices[selectedVertexIndex].position;
            float rayParam;
            float distToSelected = RayIntersection::rayPointDistance(ray, selectedPos, rayParam);

            if (distToSelected > deselectionThreshold) {
                clearSelection();
                return true; // Selection changed (cleared)
            }
        }
        return false; // No change
    }

    // Select the closest vertex
    setSelectedVertex(closestVertexIndex);
    return true; // Selection changed
}