#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "../core/Camera.h"
#include "../input/InputHandler.h"
#include "../rendering/SoftwareRenderer.h"
#include "../utils/Utils.h"
#include <iostream>
#include <vector>
#include <chrono>

class GLFWTestApp
{
private:
    GLFWwindow *window;
    Camera camera;
    InputHandler *inputHandler;
    SoftwareRenderer renderer;

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
    GLFWTestApp() : window(nullptr), inputHandler(nullptr) {}

    ~GLFWTestApp()
    {
        cleanup();
    }

    bool initialize()
    {
        // Initialize GLFW
        if (!glfwInit())
        {
            Utils::logError("Failed to initialize GLFW");
            return false;
        }

        // Configure GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Create window
        window = glfwCreateWindow(windowWidth, windowHeight,
                                  "3D Model Editor - Phase 3 Test", nullptr, nullptr);
        if (!window)
        {
            Utils::logError("Failed to create GLFW window");
            glfwTerminate();
            return false;
        }

        // Make OpenGL context current
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup camera
        camera.setAspectRatio(static_cast<float>(windowWidth) / windowHeight);
        camera.setDistance(5.0f);
        camera.setIsometricView();

        // Setup input handler
        inputHandler = new InputHandler(window, &camera);
        InputHandler::setExternalResizeCallback(onResizeStatic, this);
        inputHandler->setupCallbacks();

        // Setup renderer
        renderer.initialize();
        renderer.setResolution(windowWidth, windowHeight);

        // Create test scene
        createTestScene();

        // Resize callback is now handled by InputHandler

        // Initialize pixel buffer
        pixelBuffer.resize(windowWidth * windowHeight * 3);

        // Initialize timing
        lastFrameTime = std::chrono::steady_clock::now();

        Utils::logInfo("GLFW Test Application initialized successfully");
        printControls();

        return true;
    }

    void createTestScene()
    {
        // Create a colorful test scene with multiple triangles

        // Floor plane (gray)
        renderer.addTriangle(Triangle(
            Vector3(-2, -2, -1),
            Vector3(2, -2, -1),
            Vector3(2, 2, -1),
            Vector3(0.3f, 0.3f, 0.3f)));
        renderer.addTriangle(Triangle(
            Vector3(-2, -2, -1),
            Vector3(2, 2, -1),
            Vector3(-2, 2, -1),
            Vector3(0.4f, 0.4f, 0.4f)));

        // Standing triangle (red)
        renderer.addTriangle(Triangle(
            Vector3(-1, 0, -1),
            Vector3(1, 0, -1),
            Vector3(0, 0, 1),
            Vector3(1.0f, 0.2f, 0.2f)));

        // Side triangle (green)
        renderer.addTriangle(Triangle(
            Vector3(0, -1, -1),
            Vector3(0, -1, 1),
            Vector3(0, 1, 0),
            Vector3(0.2f, 1.0f, 0.2f)));

        // Angled triangle (blue)
        renderer.addTriangle(Triangle(
            Vector3(-0.5f, 0.5f, 0),
            Vector3(0.5f, -0.5f, 0),
            Vector3(0, 0, 1.5f),
            Vector3(0.2f, 0.2f, 1.0f)));

        Utils::logInfo("Test scene created with 5 triangles");
    }

    void run()
    {
        while (!glfwWindowShouldClose(window))
        {
            updateTiming();

            // Poll events
            glfwPollEvents();

            // Update input
            inputHandler->update();

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

    void render()
    {
        // Update renderer camera from our camera
        renderer.setCamera(camera.getPosition(), camera.getTarget(), Vector3(0, 0, 1));

        // Render the scene
        renderer.render();
    }

    void displayFrame()
    {
        const auto &pixels = renderer.getPixelData();

        // Convert float RGB to unsigned char RGB
        for (int i = 0; i < windowWidth * windowHeight; ++i)
        {
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

    void updateTiming()
    {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastFrameTime;
        deltaTime = elapsed.count();
        lastFrameTime = currentTime;
    }

    void updateFPS()
    {
        frameCount++;
        fpsTimer += deltaTime;

        if (fpsTimer >= 1.0f)
        {
            float fps = frameCount / fpsTimer;
            std::string title = "3D Model Editor - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            fpsTimer = 0.0f;
        }
    }

    void cleanup()
    {
        if (inputHandler)
        {
            delete inputHandler;
            inputHandler = nullptr;
        }

        if (window)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();

        Utils::logInfo("GLFW Test Application cleaned up");
    }

    void printControls()
    {
        std::cout << "\n===== CAMERA CONTROLS =====\n";
        std::cout << "Middle Mouse + Drag : Orbit camera\n";
        std::cout << "Mouse Wheel        : Zoom in/out\n";
        std::cout << "1 key              : Front view\n";
        std::cout << "3 key              : Right view\n";
        std::cout << "7 key              : Top view\n";
        std::cout << "5 key              : Isometric view\n";
        std::cout << "ESC                : Exit\n";
        std::cout << "===========================\n\n";
    }

    // Static callback for window resize (called by InputHandler)
    static void onResizeStatic(void *userPtr, int width, int height)
    {
        GLFWTestApp *app = static_cast<GLFWTestApp *>(userPtr);
        if (app)
        {
            app->onResize(width, height);
        }
    }

    void onResize(int width, int height)
    {
        windowWidth = width;
        windowHeight = height;

        // Update OpenGL viewport
        glViewport(0, 0, width, height);

        // Update camera aspect ratio
        camera.setAspectRatio(static_cast<float>(width) / height);

        // Update renderer resolution
        renderer.setResolution(width, height);

        // Resize pixel buffer
        pixelBuffer.resize(width * height * 3);

        Utils::logInfo("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
    }
};

int main()
{
    Utils::logInfo("Starting GLFW Integration Test");

    GLFWTestApp app;

    if (!app.initialize())
    {
        Utils::logError("Failed to initialize application");
        return -1;
    }

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        Utils::logError("Runtime error: " + std::string(e.what()));
        return -1;
    }

    Utils::logInfo("GLFW Integration Test completed successfully");
    return 0;
}