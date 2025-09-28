#include "InputHandler.h"
#include "../core/Camera.h"
#include "../core/Model.h"
#include "../core/Ray.h"
#include "../utils/Utils.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <algorithm>

// Static member definitions
void *InputHandler::externalUserPointer = nullptr;
void (*InputHandler::externalResizeCallback)(void *userPtr, int width, int height) = nullptr;

InputHandler::InputHandler(GLFWwindow *window, Camera *camera, Model *model)
    : window(window), camera(camera), currentMode(InputMode::NORMAL), model(model), baseSelectionThreshold(0.2f), lastMouseX(0.0), lastMouseY(0.0), mouseDeltaX(0.0), mouseDeltaY(0.0), firstMouse(true), orbitSensitivity(0.002f), zoomSensitivity(0.1f)
{

    std::memset(mouseButtons, false, sizeof(mouseButtons));
    std::memset(keysPressed, false, sizeof(keysPressed));

    // Store this instance in GLFW window user pointer
    glfwSetWindowUserPointer(window, this);

    // DON'T get initial mouse position here - wait for first real mouse movement
}

void InputHandler::setupCallbacks()
{
    glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCallback);
    glfwSetCursorPosCallback(window, InputHandler::cursorPosCallback);
    glfwSetScrollCallback(window, InputHandler::scrollCallback);
    glfwSetKeyCallback(window, InputHandler::keyCallback);
    glfwSetFramebufferSizeCallback(window, InputHandler::framebufferSizeCallback);

    Utils::logInfo("Input callbacks registered");
}

void InputHandler::setExternalResizeCallback(void (*callback)(void *userPtr, int width, int height), void *userPtr)
{
    externalResizeCallback = callback;
    externalUserPointer = userPtr;
}

void InputHandler::update()
{
    // Handle preset view keys only
    handlePresetViews();

    // DON'T call handleCameraOrbit here - it's handled in callbacks only
    // DON'T reset mouse deltas here - they're reset in callbacks
}

void InputHandler::setMode(InputMode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;

        // Update cursor mode based on input mode
        if (mode == InputMode::CAMERA_ORBIT)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        Utils::logInfo("Input mode changed to: " + std::to_string(static_cast<int>(mode)));
    }
}

bool InputHandler::isMouseButtonPressed(MouseButton button) const
{
    return mouseButtons[static_cast<int>(button)];
}

void InputHandler::getMouseDelta(double &deltaX, double &deltaY) const
{
    deltaX = mouseDeltaX;
    deltaY = mouseDeltaY;
}

void InputHandler::getMousePosition(double &x, double &y) const
{
    glfwGetCursorPos(window, &x, &y);
}

bool InputHandler::isKeyPressed(int key) const
{
    if (key >= 0 && key < 512)
    {
        return keysPressed[key];
    }
    return false;
}

bool InputHandler::isKeyJustPressed(int key) const
{
    // For now, same as isKeyPressed
    // Could be enhanced with previous frame state tracking
    return isKeyPressed(key);
}

void InputHandler::handleCameraOrbit()
{
    if (!camera)
        return;

    double deltaX, deltaY;
    getMouseDelta(deltaX, deltaY);

    if (deltaX != 0.0 || deltaY != 0.0)
    {
        float deltaPitch = deltaY * orbitSensitivity;
        float deltaYaw = -deltaX * orbitSensitivity;

        camera->orbit(deltaPitch, deltaYaw);
    }
}

void InputHandler::handleCameraZoom(double yoffset)
{
    if (!camera)
        return;

    // Clamp zoom factor to prevent distance from becoming 0 or negative
    float zoomFactor = 1.0f - (yoffset * zoomSensitivity);
    zoomFactor = std::max(0.1f, std::min(2.0f, zoomFactor)); // Clamp between 0.1 and 2.0

    camera->zoom(zoomFactor);
}

void InputHandler::handlePresetViews()
{
    if (!camera)
        return;

    if (isKeyPressed(GLFW_KEY_1))
    {
        camera->setFrontView();
    }
    else if (isKeyPressed(GLFW_KEY_3))
    {
        camera->setRightView();
    }
    else if (isKeyPressed(GLFW_KEY_7))
    {
        camera->setTopView();
    }
    else if (isKeyPressed(GLFW_KEY_5))
    {
        camera->setIsometricView();
    }
}

InputHandler *InputHandler::getInstance(GLFWwindow *window)
{
    return static_cast<InputHandler *>(glfwGetWindowUserPointer(window));
}

// Static callback implementations
void InputHandler::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    InputHandler *handler = getInstance(window);
    if (!handler)
        return;

    if (button >= 0 && button < 3)
    {
        handler->mouseButtons[button] = (action == GLFW_PRESS);

        // Handle mode changes based on button press
        if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (action == GLFW_PRESS)
            {
                handler->setMode(InputMode::CAMERA_ORBIT);
                // Initialize mouse position immediately to prevent jump
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                handler->lastMouseX = mouseX;
                handler->lastMouseY = mouseY;
                handler->firstMouse = true; // Set to true to ignore first movement
            }
            else if (action == GLFW_RELEASE)
            {
                handler->setMode(InputMode::NORMAL);
            }
        }
        // Handle left click for vertex selection
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            if (handler->model && handler->camera)
            {
                // Get mouse position
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);

                // Get window size for screen-to-world conversion
                int windowWidth, windowHeight;
                glfwGetWindowSize(window, &windowWidth, &windowHeight);

                // Create ray from camera through mouse position
                Ray ray = handler->camera->screenToWorldRay(mouseX, mouseY, windowWidth, windowHeight);

                // Attempt vertex selection
                bool selected = handler->model->selectVertex(ray, *handler->camera, handler->baseSelectionThreshold);

                if (selected)
                {
                    int selectedIndex = handler->model->getSelectedVertexIndex();
                    Vector3 position = handler->model->getSelectedVertexPosition();
                    Utils::logInfo("Selected vertex " + std::to_string(selectedIndex) +
                                   " at position (" + std::to_string(position.x) + "," +
                                   std::to_string(position.y) + "," + std::to_string(position.z) + ")");
                }
                else
                {
                    Utils::logInfo("No vertex selected");
                }
            }
        }
    }
}

void InputHandler::cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    InputHandler *handler = getInstance(window);
    if (!handler)
        return;

    // Handle first mouse movement - just initialize position, no camera movement
    if (handler->firstMouse)
    {
        handler->lastMouseX = xpos;
        handler->lastMouseY = ypos;
        handler->firstMouse = false;
        return; // Exit immediately, no camera processing
    }

    // Calculate deltas before updating last position
    double deltaX = xpos - handler->lastMouseX;
    double deltaY = ypos - handler->lastMouseY;

    // Update position tracking for next frame
    handler->lastMouseX = xpos;
    handler->lastMouseY = ypos;

    // ONLY process camera movement if explicitly in orbit mode AND not ignoring movement
    if (handler->currentMode == InputMode::CAMERA_ORBIT)
    {
        // Store deltas for processing
        handler->mouseDeltaX = deltaX;
        handler->mouseDeltaY = deltaY;

        // Check for abnormally large movements (probably caused by cursor mode change)
        bool isAbnormalMovement = (abs(deltaX) > 100.0 || abs(deltaY) > 100.0);
        // Check for meaningful movement (ignore tiny jitter)
        bool isMeaningfulMovement = (abs(deltaX) >= 0.5 || abs(deltaY) >= 0.5);

        if (isAbnormalMovement)
        {
            return; // Ignore this movement
        }

        // Only handle camera orbit if there's meaningful movement
        if (isMeaningfulMovement)
        {
            handler->handleCameraOrbit();
        }
    }
}

void InputHandler::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    InputHandler *handler = getInstance(window);
    if (!handler)
        return;

    handler->handleCameraZoom(yoffset);
}

void InputHandler::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    InputHandler *handler = getInstance(window);
    if (!handler)
        return;

    if (key >= 0 && key < 512)
    {
        if (action == GLFW_PRESS)
        {
            handler->keysPressed[key] = true;

            // Handle preset views immediately on key press
            if (key == GLFW_KEY_1 || key == GLFW_KEY_3 ||
                key == GLFW_KEY_7 || key == GLFW_KEY_5)
            {
                handler->handlePresetViews();
            }
        }
        else if (action == GLFW_RELEASE)
        {
            handler->keysPressed[key] = false;
        }
    }

    // Handle escape key
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void InputHandler::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    // Call external resize callback if set
    if (externalResizeCallback && externalUserPointer)
    {
        externalResizeCallback(externalUserPointer, width, height);
    }
}