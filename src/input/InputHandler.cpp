#include "InputHandler.h"
#include "../core/Camera.h"
#include "../utils/Utils.h"
#include <GLFW/glfw3.h>
#include <cstring>

InputHandler::InputHandler(GLFWwindow* window, Camera* camera)
    : window(window)
    , camera(camera)
    , currentMode(InputMode::NORMAL)
    , lastMouseX(0.0)
    , lastMouseY(0.0)
    , mouseDeltaX(0.0)
    , mouseDeltaY(0.0)
    , firstMouse(true)
    , orbitSensitivity(0.005f)
    , zoomSensitivity(0.1f) {

    std::memset(mouseButtons, false, sizeof(mouseButtons));
    std::memset(keysPressed, false, sizeof(keysPressed));

    // Store this instance in GLFW window user pointer
    glfwSetWindowUserPointer(window, this);

    // Get initial mouse position
    glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
}

void InputHandler::setupCallbacks() {
    glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCallback);
    glfwSetCursorPosCallback(window, InputHandler::cursorPosCallback);
    glfwSetScrollCallback(window, InputHandler::scrollCallback);
    glfwSetKeyCallback(window, InputHandler::keyCallback);

    Utils::logInfo("Input callbacks registered");
}

void InputHandler::update() {
    // Reset mouse delta after use
    mouseDeltaX = 0.0;
    mouseDeltaY = 0.0;

    // Handle continuous inputs
    if (currentMode == InputMode::CAMERA_ORBIT) {
        handleCameraOrbit();
    }

    // Handle preset view keys
    handlePresetViews();
}

void InputHandler::setMode(InputMode mode) {
    if (currentMode != mode) {
        currentMode = mode;

        // Update cursor mode based on input mode
        if (mode == InputMode::CAMERA_ORBIT) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        Utils::logInfo("Input mode changed to: " + std::to_string(static_cast<int>(mode)));
    }
}

bool InputHandler::isMouseButtonPressed(MouseButton button) const {
    return mouseButtons[static_cast<int>(button)];
}

void InputHandler::getMouseDelta(double& deltaX, double& deltaY) const {
    deltaX = mouseDeltaX;
    deltaY = mouseDeltaY;
}

void InputHandler::getMousePosition(double& x, double& y) const {
    glfwGetCursorPos(window, &x, &y);
}

bool InputHandler::isKeyPressed(int key) const {
    if (key >= 0 && key < 512) {
        return keysPressed[key];
    }
    return false;
}

bool InputHandler::isKeyJustPressed(int key) const {
    // For now, same as isKeyPressed
    // Could be enhanced with previous frame state tracking
    return isKeyPressed(key);
}

void InputHandler::handleCameraOrbit() {
    if (!camera) return;

    double deltaX, deltaY;
    getMouseDelta(deltaX, deltaY);

    if (deltaX != 0.0 || deltaY != 0.0) {
        camera->orbit(-deltaY * orbitSensitivity, -deltaX * orbitSensitivity);
    }
}

void InputHandler::handleCameraZoom(double yoffset) {
    if (!camera) return;

    float zoomFactor = 1.0f - (yoffset * zoomSensitivity);
    camera->zoom(zoomFactor);
}

void InputHandler::handlePresetViews() {
    if (!camera) return;

    if (isKeyPressed(GLFW_KEY_1)) {
        camera->setFrontView();
    } else if (isKeyPressed(GLFW_KEY_3)) {
        camera->setRightView();
    } else if (isKeyPressed(GLFW_KEY_7)) {
        camera->setTopView();
    } else if (isKeyPressed(GLFW_KEY_5)) {
        camera->setIsometricView();
    }
}

InputHandler* InputHandler::getInstance(GLFWwindow* window) {
    return static_cast<InputHandler*>(glfwGetWindowUserPointer(window));
}

// Static callback implementations
void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    InputHandler* handler = getInstance(window);
    if (!handler) return;

    if (button >= 0 && button < 3) {
        handler->mouseButtons[button] = (action == GLFW_PRESS);

        // Handle mode changes based on button press
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (action == GLFW_PRESS) {
                handler->setMode(InputMode::CAMERA_ORBIT);
                handler->firstMouse = true;  // Reset mouse tracking
            } else if (action == GLFW_RELEASE) {
                handler->setMode(InputMode::NORMAL);
            }
        }
    }
}

void InputHandler::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    InputHandler* handler = getInstance(window);
    if (!handler) return;

    if (handler->firstMouse) {
        handler->lastMouseX = xpos;
        handler->lastMouseY = ypos;
        handler->firstMouse = false;
        return; // Early return to avoid calculating delta on first mouse movement
    }

    handler->mouseDeltaX = xpos - handler->lastMouseX;
    handler->mouseDeltaY = ypos - handler->lastMouseY;

    handler->lastMouseX = xpos;
    handler->lastMouseY = ypos;

    // Handle camera orbit in real-time
    if (handler->currentMode == InputMode::CAMERA_ORBIT) {
        handler->handleCameraOrbit();
    }
}

void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputHandler* handler = getInstance(window);
    if (!handler) return;

    handler->handleCameraZoom(yoffset);
}

void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputHandler* handler = getInstance(window);
    if (!handler) return;

    if (key >= 0 && key < 512) {
        if (action == GLFW_PRESS) {
            handler->keysPressed[key] = true;

            // Handle preset views immediately on key press
            if (key == GLFW_KEY_1 || key == GLFW_KEY_3 ||
                key == GLFW_KEY_7 || key == GLFW_KEY_5) {
                handler->handlePresetViews();
            }
        } else if (action == GLFW_RELEASE) {
            handler->keysPressed[key] = false;
        }
    }

    // Handle escape key
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}