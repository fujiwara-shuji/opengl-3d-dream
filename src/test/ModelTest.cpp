#include <iostream>
#include "../core/Model.h"
#include "../core/Camera.h"
#include "../rendering/SoftwareRenderer.h"
#include "../utils/Utils.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <chrono>

class ModelTestApp {
private:
    GLFWwindow* window;
    Camera camera;
    SoftwareRenderer renderer;
    Model model;

    int windowWidth = 800;
    int windowHeight = 600;

    // Frame buffer for OpenGL display
    std::vector<unsigned char> pixelBuffer;

    // Frame timing
    std::chrono::steady_clock::time_point lastFrameTime;
    float deltaTime = 0.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;

public:
    ModelTestApp() : window(nullptr) {}

    ~ModelTestApp() {
        cleanup();
    }

    bool initialize() {
        // Initialize GLFW
        if (!glfwInit()) {
            Utils::logError("Failed to initialize GLFW");
            return false;
        }

        // Configure GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Create window
        window = glfwCreateWindow(windowWidth, windowHeight,
                                  "3D Model Editor - Phase 4 Test", nullptr, nullptr);
        if (!window) {
            Utils::logError("Failed to create GLFW window");
            glfwTerminate();
            return false;
        }

        // Make OpenGL context current
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup camera
        camera.setAspectRatio(static_cast<float>(windowWidth) / windowHeight);
        camera.setDistance(3.0f);
        camera.setIsometricView();

        // Setup renderer
        renderer.initialize();
        renderer.setResolution(windowWidth, windowHeight);

        // Initialize pixel buffer
        pixelBuffer.resize(windowWidth * windowHeight * 3);

        // Initialize timing
        lastFrameTime = std::chrono::steady_clock::now();

        Utils::logInfo("Model Test Application initialized successfully");
        printInstructions();

        return true;
    }

    void loadTestModel(const std::string& filename) {
        if (model.loadFromFile(filename)) {
            Utils::logInfo("Loaded model: " + filename);
            Utils::logInfo("Vertices: " + std::to_string(model.getVertexCount()));
            Utils::logInfo("Faces: " + std::to_string(model.getFaceCount()));
            Utils::logInfo("Edges: " + std::to_string(model.getEdgeCount()));
        } else {
            Utils::logError("Failed to load model: " + filename);

            // Create a default cube if loading fails
            Utils::logInfo("Creating default cube instead");
            model.createCube(1.0f);
        }
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            updateTiming();

            // Poll events
            glfwPollEvents();

            // Handle input
            handleInput();

            // Render scene
            render();

            // Display to window
            displayFrame();

            // Swap buffers
            glfwSwapBuffers(window);

            // Show FPS
            updateFPS();
        }
    }

    void render() {
        // Render the model using the new render method
        renderer.render(model, camera);
    }

    void displayFrame() {
        const auto& pixels = renderer.getPixelData();

        // Convert float RGB to unsigned char RGB
        for (int i = 0; i < windowWidth * windowHeight; ++i) {
            pixelBuffer[i * 3 + 0] = static_cast<unsigned char>(pixels[i].x * 255.0f);
            pixelBuffer[i * 3 + 1] = static_cast<unsigned char>(pixels[i].y * 255.0f);
            pixelBuffer[i * 3 + 2] = static_cast<unsigned char>(pixels[i].z * 255.0f);
        }

        // Display using OpenGL
        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2f(-1, -1);
        glPixelZoom(1, 1);
        glDrawPixels(windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer.data());
    }

    void handleInput() {
        // ESC to quit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Camera controls
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            camera.setFrontView();
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            camera.setRightView();
        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            camera.setTopView();
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
            camera.setIsometricView();
        }

        // Model loading tests
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            static bool tPressed = false;
            if (!tPressed) {
                loadTestModel("test_triangle.fjwr");
                tPressed = true;
            }
        } else {
            static bool tPressed = false;
            tPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            static bool cPressed = false;
            if (!cPressed) {
                loadTestModel("test_cube.fjwr");
                cPressed = true;
            }
        } else {
            static bool cPressed = false;
            cPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            static bool gPressed = false;
            if (!gPressed) {
                Utils::logInfo("Generating cube programmatically");
                model.createCube(1.5f);
                gPressed = true;
            }
        } else {
            static bool gPressed = false;
            gPressed = false;
        }
    }

    void updateTiming() {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastFrameTime;
        deltaTime = elapsed.count();
        lastFrameTime = currentTime;
    }

    void updateFPS() {
        frameCount++;
        fpsTimer += deltaTime;

        if (fpsTimer >= 1.0f) {
            float fps = frameCount / fpsTimer;
            std::string title = "3D Model Editor - Phase 4 - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            fpsTimer = 0.0f;
        }
    }

    void cleanup() {
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();

        Utils::logInfo("Model Test Application cleaned up");
    }

    void printInstructions() {
        std::cout << "\n===== MODEL TEST CONTROLS =====\n";
        std::cout << "1 key              : Front view\n";
        std::cout << "3 key              : Right view\n";
        std::cout << "7 key              : Top view\n";
        std::cout << "5 key              : Isometric view\n";
        std::cout << "T key              : Load test_triangle.fjwr\n";
        std::cout << "C key              : Load test_cube.fjwr\n";
        std::cout << "G key              : Generate cube programmatically\n";
        std::cout << "ESC                : Exit\n";
        std::cout << "================================\n\n";
    }
};

int main() {
    Utils::logInfo("Starting Model Test Application (Phase 4)");

    ModelTestApp app;

    if (!app.initialize()) {
        Utils::logError("Failed to initialize application");
        return -1;
    }

    // Load initial model
    app.loadTestModel("test_triangle.fjwr");

    try {
        app.run();
    } catch (const std::exception& e) {
        Utils::logError("Runtime error: " + std::string(e.what()));
        return -1;
    }

    Utils::logInfo("Model Test Application completed successfully");
    return 0;
}