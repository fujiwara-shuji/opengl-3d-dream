#include "InputHandler.h"
#include "../core/Camera.h"
#include "../utils/Utils.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <algorithm>

// Static member definitions
void* InputHandler::externalUserPointer = nullptr;
void (*InputHandler::externalResizeCallback)(void* userPtr, int width, int height) = nullptr;

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

    // DON'T get initial mouse position here - wait for first real mouse movement
}

void InputHandler::setupCallbacks() {
    glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCallback);
    glfwSetCursorPosCallback(window, InputHandler::cursorPosCallback);
    glfwSetScrollCallback(window, InputHandler::scrollCallback);
    glfwSetKeyCallback(window, InputHandler::keyCallback);
    glfwSetFramebufferSizeCallback(window, InputHandler::framebufferSizeCallback);

    Utils::logInfo("Input callbacks registered");
}

void InputHandler::setExternalResizeCallback(void (*callback)(void* userPtr, int width, int height), void* userPtr) {
    externalResizeCallback = callback;
    externalUserPointer = userPtr;
}

void InputHandler::update() {
    // Handle preset view keys only
    handlePresetViews();

    // DON'T call handleCameraOrbit here - it's handled in callbacks only
    // DON'T reset mouse deltas here - they're reset in callbacks
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

    // Clamp zoom factor to prevent distance from becoming 0 or negative
    float zoomFactor = 1.0f - (yoffset * zoomSensitivity);
    zoomFactor = std::max(0.1f, std::min(2.0f, zoomFactor));  // Clamp between 0.1 and 2.0

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

    // Only log the first mouse movement to confirm it's working
    if (handler->firstMouse) {
        Utils::logInfo("FIRST CURSOR EVENT: pos=" + std::to_string(xpos) + "," + std::to_string(ypos));
    }

    // Handle first mouse movement - just initialize position, no camera movement
    if (handler->firstMouse) {
        handler->lastMouseX = xpos;
        handler->lastMouseY = ypos;
        handler->firstMouse = false;
        Utils::logInfo("FIRST MOUSE - Position initialized, no camera processing");
        return; // CRITICAL: Exit immediately, no camera processing
    }

    // Calculate deltas before updating last position
    double deltaX = xpos - handler->lastMouseX;
    double deltaY = ypos - handler->lastMouseY;

    // Update position tracking for next frame
    handler->lastMouseX = xpos;
    handler->lastMouseY = ypos;

    // ONLY process camera movement if explicitly in orbit mode
    if (handler->currentMode == InputMode::CAMERA_ORBIT) {
        Utils::logInfo("CURSOR DELTA: " + std::to_string(deltaX) + "," + std::to_string(deltaY));
        // Store deltas for processing
        handler->mouseDeltaX = deltaX;
        handler->mouseDeltaY = deltaY;

        // Only handle camera orbit if there's actual movement
        if (deltaX != 0.0 || deltaY != 0.0) {
            handler->handleCameraOrbit();
        }
    }
}

void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputHandler* handler = getInstance(window);
    if (!handler) return;

    Utils::logInfo("SCROLL CALLBACK: xoffset=" + std::to_string(xoffset) +
                   " yoffset=" + std::to_string(yoffset));

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

void InputHandler::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Call external resize callback if set
    if (externalResizeCallback && externalUserPointer) {
        externalResizeCallback(externalUserPointer, width, height);
    }
}