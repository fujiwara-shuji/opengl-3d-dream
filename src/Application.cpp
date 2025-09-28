#include "Application.h"
#include "utils/Utils.h"

bool Application::initialize() {
    Utils::logInfo("Initializing Application...");

    // Phase 0: Basic initialization
    if (isInitialized) {
        Utils::logError("Application already initialized");
        return false;
    }

    // TODO Phase 1: Initialize GLFW
    // if (!glfwInit()) {
    //     Utils::logError("Failed to initialize GLFW");
    //     return false;
    // }

    // TODO Phase 1: Create window
    // window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, nullptr, nullptr);
    // if (!window) {
    //     Utils::logError("Failed to create GLFW window");
    //     glfwTerminate();
    //     return false;
    // }

    // TODO Phase 1: Setup OpenGL context
    // glfwMakeContextCurrent(window);
    // Utils::checkGLError("OpenGL context creation");

    // TODO Phase 1: Setup callbacks
    // setupCallbacks();

    // TODO Phase 2: Initialize renderer
    // renderer = std::make_unique<SoftwareRenderer>();
    // if (!renderer->initialize()) {
    //     Utils::logError("Failed to initialize renderer");
    //     return false;
    // }

    // TODO Phase 3: Initialize camera
    // camera = std::make_unique<Camera>();

    // TODO Phase 3: Initialize input handler
    // inputHandler = std::make_unique<InputHandler>(window);

    // TODO Phase 4: Initialize model (empty model for now)
    // model = std::make_unique<Model>();

    // TODO Phase 6: Initialize UI
    // ui = std::make_unique<UI>(window);

    isInitialized = true;
    Utils::logInfo("Application initialized successfully (Phase 0 - minimal setup)");
    return true;
}

void Application::run() {
    if (!isInitialized) {
        Utils::logError("Application not initialized");
        return;
    }

    Utils::logInfo("Starting main loop...");

    // Phase 0: Simple message loop (no actual rendering yet)
    std::cout << "Phase 0: Basic application structure created!" << std::endl;
    std::cout << "Window size: " << windowWidth << "x" << windowHeight << std::endl;
    std::cout << "Ready for Phase 1 implementation (GLFW + OpenGL setup)" << std::endl;

    // TODO Phase 1+: Replace with actual main loop
    // while (!glfwWindowShouldClose(window) && !shouldClose) {
    //     update();
    //     render();
    //     glfwSwapBuffers(window);
    //     glfwPollEvents();
    // }

    Utils::logInfo("Main loop finished");
}

void Application::shutdown() {
    if (!isInitialized) {
        return;
    }

    Utils::logInfo("Shutting down Application...");

    // TODO Phase 6: Cleanup UI
    // ui.reset();

    // TODO Phase 4: Cleanup model
    // model.reset();

    // TODO Phase 3: Cleanup input handler
    // inputHandler.reset();

    // TODO Phase 3: Cleanup camera
    // camera.reset();

    // TODO Phase 2: Cleanup renderer
    // renderer.reset();

    // TODO Phase 1: Cleanup GLFW
    // if (window) {
    //     glfwDestroyWindow(window);
    //     window = nullptr;
    // }
    // glfwTerminate();

    isInitialized = false;
    Utils::logInfo("Application shutdown complete");
}

void Application::setupCallbacks() {
    // TODO Phase 1: Implement GLFW callbacks
    // glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    // glfwSetErrorCallback(errorCallback);
    // glfwSetWindowUserPointer(window, this);
}

void Application::update() {
    // TODO Phase 3+: Implement update logic
    // - Handle input
    // - Update camera
    // - Update model state
}

void Application::render() {
    // TODO Phase 2+: Implement rendering
    // - Clear screen
    // - Render model using software raytracer
    // - Render UI
}

void Application::framebufferSizeCallback(void* window, int width, int height) {
    // TODO Phase 1: Implement framebuffer resize callback
    // Application* app = static_cast<Application*>(glfwGetWindowUserPointer(static_cast<GLFWwindow*>(window)));
    // if (app) {
    //     app->windowWidth = width;
    //     app->windowHeight = height;
    //     if (app->renderer) {
    //         app->renderer->setResolution(width, height);
    //     }
    // }
    Utils::logInfo("Framebuffer resize callback (not implemented yet)");
}

void Application::errorCallback(int error, const char* description) {
    Utils::logError("GLFW Error " + std::to_string(error) + ": " + description);
}