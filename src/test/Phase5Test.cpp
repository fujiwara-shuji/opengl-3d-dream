#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include "../core/Camera.h"
#include "../core/Model.h"
#include "../core/CoordinateAxes.h"
#include "../input/InputHandler.h"
#include "../rendering/SoftwareRenderer.h"
#include "../ui/UI.h"
#include "../utils/Utils.h"
#include <iostream>
#include <vector>
#include <chrono>

class Phase5TestApp
{
private:
    GLFWwindow *window;
    Camera camera;
    Model model;
    CoordinateAxes coordinateAxes;
    InputHandler *inputHandler;
    SoftwareRenderer renderer;
    UI ui;

    int windowWidth = 1000;
    int windowHeight = 800;

    // Frame buffer for OpenGL display
    std::vector<unsigned char> pixelBuffer;

    // Frame timing
    std::chrono::steady_clock::time_point lastFrameTime;
    float deltaTime = 0.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;

public:
    Phase5TestApp() : window(nullptr), inputHandler(nullptr) {}

    ~Phase5TestApp()
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
                                  "Phase 5 Test - Vertex & Edge Selection", nullptr, nullptr);
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

        // Create test model
        createTestModel();

        // Setup input handler with model
        inputHandler = new InputHandler(window, &camera, &model);
        InputHandler::setExternalResizeCallback(onResizeStatic, this);
        inputHandler->setupCallbacks();

        // Setup renderer
        renderer.initialize();
        renderer.setResolution(windowWidth, windowHeight);

        // Load model into renderer
        loadModelIntoRenderer();

        // Initialize pixel buffer
        pixelBuffer.resize(windowWidth * windowHeight * 3);

        // Initialize timing
        lastFrameTime = std::chrono::steady_clock::now();

        // Initialize UI
        ui.setCamera(&camera);
        ui.setModel(&model);
        ui.setCoordinateAxes(&coordinateAxes);
        ui.setRenderer(&renderer);
        ui.setWindowSize(windowWidth, windowHeight);
        if (!ui.initialize(window)) {
            Utils::logError("Failed to initialize UI");
            return false;
        }

        // Apply initial display settings from UI to renderer
        renderer.setShowVertices(true);
        renderer.setShowEdges(true);
        renderer.setShowFaces(true);
        renderer.setShowCoordinateAxes(coordinateAxes.isVisible());

        // Configure separated display and selection thresholds
        renderer.setVertexDisplayRadius(0.015f);         // Small visual display radius
        renderer.setVertexSelectionThreshold(0.05f);     // Larger click selection range
        renderer.setEdgeDisplayThickness(0.01f);         // Thin visual edge thickness
        renderer.setEdgeSelectionThreshold(0.02f);       // Wider click selection range

        // Enable debug mode for easier vertex selection (disable visibility check)
        model.setDisableVisibilityCheck(true);
        Utils::logInfo("Visibility check disabled for easier vertex selection");

        Utils::logInfo("Phase 5 Test Application initialized successfully");
        printControls();

        return true;
    }

    void createTestModel()
    {
        Utils::logInfo("Creating test model for Phase 5...");

        // Create a simple pyramid model for testing
        model.clear();

        // Base vertices (square base)
        model.addVertex(-1.0f, -1.0f, 0.0f); // 0: Bottom-left
        model.addVertex(1.0f, -1.0f, 0.0f);  // 1: Bottom-right
        model.addVertex(1.0f, 1.0f, 0.0f);   // 2: Top-right
        model.addVertex(-1.0f, 1.0f, 0.0f);  // 3: Top-left
        model.addVertex(0.0f, 0.0f, 2.0f);   // 4: Apex

        // Test rectangle at z=-0.5
        model.addVertex(0.0f, 0.0f, -0.5f);  // 5: Rectangle bottom-left
        model.addVertex(0.5f, 0.0f, -0.5f);  // 6: Rectangle bottom-right
        model.addVertex(0.5f, 10.0f, -0.5f); // 7: Rectangle top-right
        model.addVertex(0.0f, 10.0f, -0.5f); // 8: Rectangle top-left

        // Base faces (two triangles for the square base)
        model.addFace(0, 1, 2); // Bottom triangle 1
        model.addFace(0, 2, 3); // Bottom triangle 2

        // Side faces
        model.addFace(0, 4, 1); // Side 1
        model.addFace(1, 4, 2); // Side 2
        model.addFace(2, 4, 3); // Side 3
        model.addFace(3, 4, 0); // Side 4

        // Test rectangle faces (two triangles)
        model.addFace(5, 6, 7); // Rectangle triangle 1
        model.addFace(5, 7, 8); // Rectangle triangle 2

        // Edges (manual addition for testing)
        // Base edges
        model.addEdge(0, 1);
        model.addEdge(1, 2);
        model.addEdge(2, 3);
        model.addEdge(3, 0);
        // Apex edges
        model.addEdge(0, 4);
        model.addEdge(1, 4);
        model.addEdge(2, 4);
        model.addEdge(3, 4);
        // Rectangle edges
        model.addEdge(5, 6);
        model.addEdge(6, 7);
        model.addEdge(7, 8);
        model.addEdge(8, 5);

        Utils::logInfo("Test pyramid with rectangle created:");
        Utils::logInfo("  Vertices: " + std::to_string(model.getVertexCount()));
        Utils::logInfo("  Faces: " + std::to_string(model.getFaceCount()));
        Utils::logInfo("  Edges: " + std::to_string(model.getEdgeCount()));
    }

    void loadModelIntoRenderer()
    {
        // Clear existing triangles
        renderer.clearTriangles();

        const auto &vertices = model.getVertices();
        const auto &faces = model.getFaces();

        // Add faces to renderer as triangles
        for (const auto &face : faces)
        {
            Vector3 v0 = vertices[face.v1].position;
            Vector3 v1 = vertices[face.v2].position;
            Vector3 v2 = vertices[face.v3].position;

            // Use different colors for different faces
            Vector3 color;
            if (face.v1 == 0 && face.v2 == 1 && face.v3 == 2)
            {
                color = Vector3(0.8f, 0.3f, 0.3f); // Red base
            }
            else if (face.v1 == 0 && face.v2 == 2 && face.v3 == 3)
            {
                color = Vector3(0.8f, 0.3f, 0.3f); // Red base
            }
            else if ((face.v1 == 5 && face.v2 == 6 && face.v3 == 7) ||
                     (face.v1 == 5 && face.v2 == 7 && face.v3 == 8))
            {
                color = Vector3(0.3f, 0.4f, 0.8f); // Blue rectangle
            }
            else
            {
                color = Vector3(0.4f, 0.7f, 0.4f); // Green sides
            }

            renderer.addTriangle(Triangle(v0, v1, v2, color));
        }

        // Load vertices for rendering
        std::vector<Vector3> vertexPositions;
        for (const auto &vertex : vertices)
        {
            vertexPositions.push_back(vertex.position);
        }

        // Add origin point for reference
        vertexPositions.push_back(coordinateAxes.getOriginPoint());

        renderer.setVertices(vertexPositions);

        // Load edges for rendering
        const auto &edges = model.getEdges();
        std::vector<Line> edgeLines;
        for (const auto &edge : edges)
        {
            Vector3 start = vertices[edge.v1].position;
            Vector3 end = vertices[edge.v2].position;
            Vector3 edgeColor(0.9f, 0.9f, 0.9f); // Brighter gray for visibility
            edgeLines.emplace_back(start, end, edgeColor, 1.0f);
        }
        renderer.setEdges(edgeLines);

        // Load coordinate axes
        renderer.setLines(coordinateAxes.getAxisLines());

        Utils::logInfo("Model loaded into renderer with " + std::to_string(faces.size()) + " triangles");
        Utils::logInfo("Vertices loaded: " + std::to_string(vertices.size()));
        Utils::logInfo("Edges loaded: " + std::to_string(edges.size()));
        Utils::logInfo("Coordinate axes loaded with " + std::to_string(coordinateAxes.getAxisLines().size()) + " lines");
    }

    void run()
    {
        while (!glfwWindowShouldClose(window))
        {
            updateTiming();

            // Poll events
            glfwPollEvents();

            // Handle keyboard input
            handleKeyInput();

            // Update input
            inputHandler->update();

            // Render scene
            render();

            // Start UI frame
            ui.newFrame();

            // Render UI
            ui.render();

            // Display to window
            displayFrame();

            // End UI frame (renders UI to screen)
            ui.endFrame();

            // Swap buffers
            glfwSwapBuffers(window);

            // Show FPS
            updateFPS();
        }
    }

    void handleKeyInput()
    {
        // R key: Reset camera to isometric view
        if (inputHandler->isKeyPressed(GLFW_KEY_R))
        {
            camera.setIsometricView();
            Utils::logInfo("Camera reset to isometric view");
        }

        // C key: Clear selection
        if (inputHandler->isKeyPressed(GLFW_KEY_C))
        {
            model.clearSelection();
            Utils::logInfo("Selection cleared");
        }

        // I key: Print model info
        if (inputHandler->isKeyPressed(GLFW_KEY_I))
        {
            printModelInfo();
        }

        // S key: Show selection info
        if (inputHandler->isKeyPressed(GLFW_KEY_S))
        {
            showSelectionInfo();
        }


        // A key: Toggle coordinate axes
        if (inputHandler->isKeyPressed(GLFW_KEY_A))
        {
            static bool axesVisible = true;
            axesVisible = !axesVisible;
            coordinateAxes.setVisible(axesVisible);
            coordinateAxes.regenerateAxes();
            renderer.setLines(coordinateAxes.getAxisLines());
            Utils::logInfo(axesVisible ? "Coordinate axes enabled" : "Coordinate axes disabled");
        }

        // Plus/Minus keys: Adjust axis length
        if (inputHandler->isKeyPressed(GLFW_KEY_EQUAL))
        { // + key
            float currentLength = coordinateAxes.getAxisLength();
            coordinateAxes.setAxisLength(currentLength + 0.5f);
            renderer.setLines(coordinateAxes.getAxisLines());
        }
        if (inputHandler->isKeyPressed(GLFW_KEY_MINUS))
        { // - key
            float currentLength = coordinateAxes.getAxisLength();
            if (currentLength > 0.5f)
            {
                coordinateAxes.setAxisLength(currentLength - 0.5f);
                renderer.setLines(coordinateAxes.getAxisLines());
            }
        }
    }

    void render()
    {
        // Update renderer camera from our camera
        renderer.setCamera(camera.getPosition(), camera.getTarget(), camera.getUpVector());

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

    void printModelInfo()
    {
        std::cout << "\n===== MODEL INFO =====\n";
        std::cout << "Vertices: " << model.getVertexCount() << "\n";
        std::cout << "Faces: " << model.getFaceCount() << "\n";
        std::cout << "Edges: " << model.getEdgeCount() << "\n";

        const auto &vertices = model.getVertices();
        std::cout << "\nVertex positions:\n";
        for (int i = 0; i < model.getVertexCount(); ++i)
        {
            const auto &pos = vertices[i].position;
            std::cout << "  " << i << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
        }
        std::cout << "======================\n\n";
    }

    void showSelectionInfo()
    {
        if (model.hasSelection())
        {
            int selectedIndex = model.getSelectedVertexIndex();
            Vector3 position = model.getSelectedVertexPosition();
            std::cout << "\n===== SELECTION INFO =====\n";
            std::cout << "Selected vertex: " << selectedIndex << "\n";
            std::cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
            std::cout << "==========================\n\n";
        }
        else
        {
            std::cout << "\nNo vertex selected\n\n";
        }
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
            std::string title = "Phase 5 Test - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            fpsTimer = 0.0f;
        }
    }

    void cleanup()
    {
        // Shutdown UI
        ui.shutdown();

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

        Utils::logInfo("Phase 5 Test Application cleaned up");
    }

    void printControls()
    {
        std::cout << "\n===== PHASE 5 TEST CONTROLS =====\n";
        std::cout << "LEFT CLICK         : Select vertex (with visibility check)\n";
        std::cout << "MIDDLE CLICK + DRAG: Orbit camera\n";
        std::cout << "MOUSE WHEEL        : Zoom in/out\n";
        std::cout << "1 KEY              : Front view\n";
        std::cout << "3 KEY              : Right view\n";
        std::cout << "7 KEY              : Top view\n";
        std::cout << "5 KEY              : Isometric view\n";
        std::cout << "R KEY              : Reset to isometric view\n";
        std::cout << "C KEY              : Clear selection\n";
        std::cout << "I KEY              : Print model info\n";
        std::cout << "S KEY              : Show selection info\n";
        std::cout << "A KEY              : Toggle coordinate axes\n";
        std::cout << "+ KEY              : Increase axis length\n";
        std::cout << "- KEY              : Decrease axis length\n";
        std::cout << "ESC                : Exit\n";
        std::cout << "==================================\n\n";
    }

    // Static callback for window resize (called by InputHandler)
    static void onResizeStatic(void *userPtr, int width, int height)
    {
        Phase5TestApp *app = static_cast<Phase5TestApp *>(userPtr);
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

        // Update UI window size
        ui.setWindowSize(width, height);

        Utils::logInfo("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
    }
};

int main()
{
    Utils::logInfo("Starting Phase 5 Test - Vertex & Edge Selection");

    Phase5TestApp app;

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

    Utils::logInfo("Phase 5 Test completed successfully");
    return 0;
}