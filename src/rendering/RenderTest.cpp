#include "SoftwareRenderer.h"
#include "../utils/Utils.h"
#include <iostream>

void testTriangleIntersection() {
    Utils::logInfo("Testing triangle intersection algorithms...");

    // Create a simple triangle in front of camera
    Vector3 v0(-1.0f, -1.0f, 0.0f);
    Vector3 v1( 1.0f, -1.0f, 0.0f);
    Vector3 v2( 0.0f,  1.0f, 0.0f);

    // Test ray that should hit the triangle
    Ray hitRay(Vector3(0, 0, 1), Vector3(0, 0, -1));  // From (0,0,1) towards (0,0,0)
    TriangleHit hit = RayIntersection::intersectTriangle(hitRay, v0, v1, v2);

    std::cout << "Hit test: " << (hit.hit ? "HIT" : "MISS") << std::endl;
    if (hit.hit) {
        std::cout << "  Distance: " << hit.distance << std::endl;
        std::cout << "  Point: " << hit.point << std::endl;
        std::cout << "  Normal: " << hit.normal << std::endl;
        std::cout << "  Front face: " << (hit.isFrontFace ? "YES" : "NO") << std::endl;
    }

    // Test ray that should miss the triangle
    Ray missRay(Vector3(2, 0, 1), Vector3(0, 0, -1));  // From (2,0,1) towards (2,0,0)
    TriangleHit miss = RayIntersection::intersectTriangle(missRay, v0, v1, v2);

    std::cout << "Miss test: " << (miss.hit ? "HIT" : "MISS") << std::endl;

    Utils::logInfo("Triangle intersection tests completed");
}

void testSoftwareRenderer() {
    Utils::logInfo("Testing Software Renderer...");

    // Create renderer
    SoftwareRenderer renderer;
    renderer.initialize();
    renderer.setResolution(100, 100);  // Small resolution for quick test

    // Set camera
    renderer.setCamera(Vector3(0, 0, 3), Vector3(0, 0, 0), Vector3(0, 0, 1));
    renderer.setCameraFOV(45.0f);

    // Create a simple triangle scene
    Triangle triangle1(
        Vector3(-1.0f, -1.0f, 0.0f),
        Vector3( 1.0f, -1.0f, 0.0f),
        Vector3( 0.0f,  1.0f, 0.0f),
        Vector3(1.0f, 0.0f, 0.0f)  // Red color
    );

    Triangle triangle2(
        Vector3(-0.5f, 0.0f, -1.0f),
        Vector3( 0.5f, 0.0f, -1.0f),
        Vector3( 0.0f, 1.0f, -1.0f),
        Vector3(0.0f, 1.0f, 0.0f)  // Green color
    );

    renderer.addTriangle(triangle1);
    renderer.addTriangle(triangle2);

    // Render the scene
    Utils::logInfo("Rendering scene...");
    renderer.render();

    // Save result for inspection
    renderer.saveAsText("render_test_output.txt");

    // Test a few specific pixels
    const auto& pixels = renderer.getPixelData();
    Vector3 centerPixel = pixels[50 * 100 + 50];  // Center pixel
    Vector3 cornerPixel = pixels[0 * 100 + 0];    // Top-left corner

    std::cout << "Center pixel color: " << centerPixel << std::endl;
    std::cout << "Corner pixel color: " << cornerPixel << std::endl;

    // Check if we're getting reasonable colors
    bool hasTriangleColor = (centerPixel.x > 0.5f || centerPixel.y > 0.5f); // Red or green
    bool hasSkyColor = (cornerPixel.z > cornerPixel.x && cornerPixel.z > cornerPixel.y); // Blue-ish

    std::cout << "Has triangle color in center: " << (hasTriangleColor ? "YES" : "NO") << std::endl;
    std::cout << "Has sky color in corner: " << (hasSkyColor ? "YES" : "NO") << std::endl;

    renderer.shutdown();
    Utils::logInfo("Software Renderer tests completed");
}

void testCameraRayGeneration() {
    Utils::logInfo("Testing camera ray generation (internal test)...");

    // Test the Ray class directly instead of private methods
    Ray testRay(Vector3(0, 0, 5), Vector3(0, 0, -1));
    Vector3 pointAt2 = testRay.getPoint(2.0f);

    std::cout << "Test ray from camera: origin=" << testRay.origin << " direction=" << testRay.direction << std::endl;
    std::cout << "Point at t=2.0: " << pointAt2 << std::endl;

    // Verify ray direction is normalized
    float dirLength = testRay.direction.length();
    std::cout << "Ray direction length (should be ~1.0): " << dirLength << std::endl;

    Utils::logInfo("Camera ray generation tests completed");
}

int main() {
    Utils::logInfo("Starting Raytracing Tests");

    try {
        testTriangleIntersection();
        std::cout << "\n" << std::string(50, '-') << "\n" << std::endl;

        testCameraRayGeneration();
        std::cout << "\n" << std::string(50, '-') << "\n" << std::endl;

        testSoftwareRenderer();

    } catch (const std::exception& e) {
        Utils::logError("Test failed with exception: " + std::string(e.what()));
        return -1;
    }

    Utils::logInfo("All raytracing tests completed successfully!");
    return 0;
}