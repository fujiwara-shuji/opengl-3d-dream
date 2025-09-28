#pragma once

#include <memory>
#include <iostream>
#include "math/Vector3.h"
#include "math/Matrix4.h"
#include "core/Ray.h"

// Forward declarations (will be replaced with actual headers in later phases)
// #include <GLFW/glfw3.h>  // Phase 1
// #include "core/Model.h"   // Phase 4
// #include "core/Camera.h"  // Phase 3
// #include "rendering/IRenderer.h"  // Phase 2
// #include "input/InputHandler.h"   // Phase 3
// #include "ui/UI.h"        // Phase 6

class Application {
private:
    // Window management (Phase 1)
    // GLFWwindow* window = nullptr;

    // Core components (will be added in respective phases)
    // std::unique_ptr<Model> model;           // Phase 4
    // std::unique_ptr<Camera> camera;         // Phase 3
    // std::unique_ptr<IRenderer> renderer;    // Phase 2
    // std::unique_ptr<InputHandler> inputHandler; // Phase 3
    // std::unique_ptr<UI> ui;                 // Phase 6

    // Window properties
    int windowWidth = 1280;
    int windowHeight = 720;
    const char* windowTitle = "3D Model Editor";

    // Application state
    bool shouldClose = false;
    bool isInitialized = false;

public:
    Application() = default;
    ~Application() = default;

    // Non-copyable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Main application lifecycle
    bool initialize();
    void run();
    void shutdown();

private:
    // Internal methods (to be implemented in respective phases)
    void setupCallbacks();  // Phase 1
    void update();          // Phase 3+
    void render();          // Phase 2+

    // GLFW callback functions (Phase 1)
    static void framebufferSizeCallback(void* window, int width, int height);
    static void errorCallback(int error, const char* description);
};