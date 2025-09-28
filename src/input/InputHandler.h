#pragma once

#include "../math/Vector3.h"

// Forward declarations
class Camera;
struct GLFWwindow;

class InputHandler {
public:
    enum class MouseButton {
        LEFT = 0,
        RIGHT = 1,
        MIDDLE = 2
    };

    enum class InputMode {
        NORMAL,
        CAMERA_ORBIT,
        VERTEX_SELECTION,
        VERTEX_MOVE
    };

private:
    GLFWwindow* window;
    Camera* camera;
    InputMode currentMode;

    // Model selection support
    class Model* model;
    float baseSelectionThreshold;

    // Resize callback support for GLFWTestApp
    static void* externalUserPointer;
    static void (*externalResizeCallback)(void* userPtr, int width, int height);

    // Mouse state
    bool mouseButtons[3];  // Left, Right, Middle
    double lastMouseX, lastMouseY;
    double mouseDeltaX, mouseDeltaY;
    bool firstMouse;

    // Camera control parameters
    float orbitSensitivity;
    float zoomSensitivity;

    // Key states
    bool keysPressed[512];  // GLFW key codes

public:
    InputHandler(GLFWwindow* window, Camera* camera, class Model* model = nullptr);
    ~InputHandler() = default;

    // Model selection
    void setModel(class Model* newModel) { model = newModel; }
    void setSelectionThreshold(float threshold) { baseSelectionThreshold = threshold; }

    // Set external resize callback and user pointer
    static void setExternalResizeCallback(void (*callback)(void* userPtr, int width, int height), void* userPtr);

    // Initialize GLFW callbacks
    void setupCallbacks();

    // Update per frame
    void update();

    // Input mode management
    void setMode(InputMode mode);
    InputMode getMode() const { return currentMode; }

    // Mouse state queries
    bool isMouseButtonPressed(MouseButton button) const;
    void getMouseDelta(double& deltaX, double& deltaY) const;
    void getMousePosition(double& x, double& y) const;

    // Key state queries
    bool isKeyPressed(int key) const;
    bool isKeyJustPressed(int key) const;

    // Settings
    void setOrbitSensitivity(float sensitivity) { orbitSensitivity = sensitivity; }
    void setZoomSensitivity(float sensitivity) { zoomSensitivity = sensitivity; }

    // GLFW callback handlers (static)
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

private:
    // Camera controls
    void handleCameraOrbit();
    void handleCameraZoom(double yoffset);
    void handlePresetViews();

    // Get instance from GLFW window user pointer
    static InputHandler* getInstance(GLFWwindow* window);
};