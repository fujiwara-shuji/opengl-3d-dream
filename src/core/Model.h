#pragma once

#include "../math/Vector3.h"
#include <vector>
#include <string>

// Data structures for 3D model representation
struct Vertex {
    Vector3 position;
    Vector3 normal;  // Will be calculated from faces

    Vertex() : position(0, 0, 0), normal(0, 0, 1) {}
    Vertex(const Vector3& pos) : position(pos), normal(0, 0, 1) {}
    Vertex(float x, float y, float z) : position(x, y, z), normal(0, 0, 1) {}
};

struct Face {
    int v1, v2, v3;  // Vertex indices (0-based)

    Face() : v1(0), v2(0), v3(0) {}
    Face(int vertex1, int vertex2, int vertex3) : v1(vertex1), v2(vertex2), v3(vertex3) {}
};

struct Edge {
    int v1, v2;      // Vertex indices (0-based)

    Edge() : v1(0), v2(0) {}
    Edge(int vertex1, int vertex2) : v1(vertex1), v2(vertex2) {}
};

class Model {
private:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Edge> edges;

    std::string filename;
    bool isModified;

public:
    Model();
    ~Model() = default;

    // File I/O operations
    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath);
    bool saveAs(const std::string& filePath);

    // Data access
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<Face>& getFaces() const { return faces; }
    const std::vector<Edge>& getEdges() const { return edges; }

    // Data modification
    void addVertex(const Vertex& vertex);
    void addVertex(const Vector3& position);
    void addVertex(float x, float y, float z);

    void addFace(const Face& face);
    void addFace(int v1, int v2, int v3);

    void addEdge(const Edge& edge);
    void addEdge(int v1, int v2);

    // Data removal
    void removeVertex(int index);
    void removeFace(int index);
    void removeEdge(int index);

    // Data modification
    void setVertexPosition(int index, const Vector3& position);
    Vector3 getVertexPosition(int index) const;

    // Utility functions
    void clear();
    void calculateNormals();  // Calculate vertex normals from faces

    // Model info
    int getVertexCount() const { return static_cast<int>(vertices.size()); }
    int getFaceCount() const { return static_cast<int>(faces.size()); }
    int getEdgeCount() const { return static_cast<int>(edges.size()); }

    const std::string& getFilename() const { return filename; }
    bool getIsModified() const { return isModified; }
    void setModified(bool modified) { isModified = modified; }

    // Basic shape generation
    void createCube(float size = 1.0f);
    void createTriangle();
    void createQuad();

    // Validation
    bool isValid() const;
    bool isVertexIndexValid(int index) const;
    bool isFaceValid(const Face& face) const;
    bool isEdgeValid(const Edge& edge) const;

private:
    // Internal file format parsing
    bool parseFjwrFile(const std::string& content);
    std::string generateFjwrContent() const;

    // Auto-generate edges from faces
    void generateEdgesFromFaces();

    // Mark as modified
    void markAsModified() { isModified = true; }
};