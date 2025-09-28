#pragma once

#include "../core/Camera.h"
#include "../core/Model.h"
#include "../core/CoordinateAxes.h"
#include "../rendering/SoftwareRenderer.h"

// Forward declaration for ImGui (when available)
struct ImGuiIO;
struct ImDrawData;

class UI {
private:
    // References to main components
    Camera* camera;
    Model* model;
    CoordinateAxes* coordinateAxes;
    SoftwareRenderer* renderer;

    // UI state
    bool showUI;
    bool showModelInfo;
    bool showSelectionInfo;
    bool showDisplaySettings;
    bool showAxesSettings;

    // Display settings
    bool displayVertices;
    bool displayEdges;
    bool displayFaces;
    bool displayAxes;

    // Window size for layout
    int windowWidth;
    int windowHeight;

public:
    UI();
    ~UI() = default;

    // Initialization and shutdown
    bool initialize(void* glfwWindow);
    void shutdown();

    // Component references
    void setCamera(Camera* cam) { camera = cam; }
    void setModel(Model* mdl) { model = mdl; }
    void setCoordinateAxes(CoordinateAxes* axes) { coordinateAxes = axes; }
    void setRenderer(SoftwareRenderer* rend) { renderer = rend; }

    // UI state
    void setVisible(bool visible) { showUI = visible; }
    bool isVisible() const { return showUI; }

    // Window size management
    void setWindowSize(int width, int height);

    // Main UI rendering
    void render();

    // ImGui implementation (stub for now)
    void newFrame();
    void endFrame();

private:
    // Panel rendering methods
    void renderMainMenuBar();
    void renderToolPanel();
    void renderModelInfoPanel();
    void renderSelectionInfoPanel();
    void renderDisplaySettingsPanel();
    void renderAxesSettingsPanel();

    // Helper methods
    void applyDisplaySettings();
    void applyAxesSettings();

    // ImGui availability flag
    bool imguiAvailable;
};