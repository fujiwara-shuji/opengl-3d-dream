#pragma once

#include <vector>

// Forward declarations
struct Vector3;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    // Core rendering methods
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual void setResolution(int width, int height) = 0;

    // Render a frame
    virtual void render() = 0;

    // Get the rendered pixel data (RGB format)
    virtual const std::vector<Vector3>& getPixelData() const = 0;

    // Clear the screen
    virtual void clear(const Vector3& clearColor) = 0;
};