#include "UI.h"
#include "../utils/Utils.h"
#include <iostream>

// ImGui includes (conditional compilation)
#ifdef IMGUI_AVAILABLE
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#endif

UI::UI()
    : camera(nullptr)
    , model(nullptr)
    , coordinateAxes(nullptr)
    , renderer(nullptr)
    , showUI(true)
    , showModelInfo(true)
    , showSelectionInfo(true)
    , showDisplaySettings(true)
    , showAxesSettings(true)
    , displayVertices(true)
    , displayEdges(true)
    , displayFaces(true)
    , displayAxes(true)
    , windowWidth(1000)
    , windowHeight(800)
    , imguiAvailable(false)
{
    #ifdef IMGUI_AVAILABLE
    imguiAvailable = true;
    #endif
}

bool UI::initialize(void* glfwWindow) {
    #ifdef IMGUI_AVAILABLE
    if (!imguiAvailable) {
        Utils::logInfo("ImGui not available, UI will use simple console output");
        return true;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(glfwWindow), true);
    ImGui_ImplOpenGL2_Init();

    Utils::logInfo("Dear ImGui initialized successfully");
    return true;
    #else
    Utils::logInfo("Dear ImGui not available, using console-based UI");
    return true;
    #endif
}

void UI::shutdown() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        Utils::logInfo("Dear ImGui shut down");
    }
    #endif
}

void UI::setWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void UI::newFrame() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable) {
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    #endif
}

void UI::endFrame() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable) {
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }
    #endif
}

void UI::render() {
    if (!showUI) return;

    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable) {
        renderMainMenuBar();
        renderToolPanel();
    } else {
        // Fallback to console output for key information
        static int frameCount = 0;
        frameCount++;

        // Print UI information every 300 frames (roughly every 10 seconds at 30fps)
        if (frameCount % 300 == 0) {
            std::cout << "\n===== UI STATUS (Frame " << frameCount << ") =====\n";
            if (model) {
                std::cout << "Model: " << model->getVertexCount() << " vertices, "
                         << model->getFaceCount() << " faces\n";
                if (model->hasSelection()) {
                    Vector3 pos = model->getSelectedVertexPosition();
                    std::cout << "Selected vertex at (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
                }
            }
            if (coordinateAxes) {
                std::cout << "Axes: " << (coordinateAxes->isVisible() ? "visible" : "hidden")
                         << ", length=" << coordinateAxes->getAxisLength() << "\n";
            }
            std::cout << "=============================================\n\n";
        }
    }
    #else
    // Simple console-based status updates
    static int frameCount = 0;
    frameCount++;

    if (frameCount % 300 == 0) {
        std::cout << "\n===== UI STATUS =====\n";
        if (model) {
            std::cout << "Model: " << model->getVertexCount() << " vertices\n";
        }
        std::cout << "===================\n\n";
    }
    #endif
}

void UI::renderMainMenuBar() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable && ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                if (model) {
                    model->clear();
                    Utils::logInfo("New model created");
                }
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                Utils::logInfo("Open dialog requested (not implemented yet)");
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                Utils::logInfo("Save requested (not implemented yet)");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                Utils::logInfo("Exit requested from menu");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Model Info", nullptr, &showModelInfo);
            ImGui::MenuItem("Selection Info", nullptr, &showSelectionInfo);
            ImGui::MenuItem("Display Settings", nullptr, &showDisplaySettings);
            ImGui::MenuItem("Axes Settings", nullptr, &showAxesSettings);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Controls")) {
                Utils::logInfo("Controls help requested");
            }
            if (ImGui::MenuItem("About")) {
                Utils::logInfo("About dialog requested");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    #endif
}

void UI::renderToolPanel() {
    #ifdef IMGUI_AVAILABLE
    if (!imguiAvailable) return;

    // Tool panel on the right side
    const float panelWidth = 300.0f;
    ImGui::SetNextWindowPos(ImVec2(windowWidth - panelWidth, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, windowHeight - 40), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Tool Panel", nullptr, ImGuiWindowFlags_NoMove)) {
        if (showModelInfo) {
            renderModelInfoPanel();
        }

        if (showSelectionInfo) {
            renderSelectionInfoPanel();
        }

        if (showDisplaySettings) {
            renderDisplaySettingsPanel();
        }

        if (showAxesSettings) {
            renderAxesSettingsPanel();
        }
    }
    ImGui::End();
    #endif
}

void UI::renderModelInfoPanel() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable && ImGui::CollapsingHeader("Model Information")) {
        if (model) {
            ImGui::Text("Vertices: %d", model->getVertexCount());
            ImGui::Text("Faces: %d", model->getFaceCount());
            ImGui::Text("Edges: %d", model->getEdgeCount());
            ImGui::Text("File: %s", model->getFilename().c_str());
            ImGui::Text("Modified: %s", model->getIsModified() ? "Yes" : "No");
        } else {
            ImGui::Text("No model loaded");
        }
    }
    #endif
}

void UI::renderSelectionInfoPanel() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable && ImGui::CollapsingHeader("Selection Information")) {
        if (model && model->hasSelection()) {
            int selectedIndex = model->getSelectedVertexIndex();
            Vector3 position = model->getSelectedVertexPosition();

            ImGui::Text("Selected Vertex: %d", selectedIndex);
            ImGui::Text("Position: (%.3f, %.3f, %.3f)", position.x, position.y, position.z);

            if (ImGui::Button("Clear Selection")) {
                model->clearSelection();
            }
        } else {
            ImGui::Text("No vertex selected");
            ImGui::Text("Left-click on a vertex to select");
        }
    }
    #endif
}

void UI::renderDisplaySettingsPanel() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable && ImGui::CollapsingHeader("Display Settings")) {
        bool changed = false;

        changed |= ImGui::Checkbox("Show Vertices", &displayVertices);
        changed |= ImGui::Checkbox("Show Edges", &displayEdges);
        changed |= ImGui::Checkbox("Show Faces", &displayFaces);

        if (changed) {
            applyDisplaySettings();
        }
    }
    #endif
}

void UI::renderAxesSettingsPanel() {
    #ifdef IMGUI_AVAILABLE
    if (imguiAvailable && ImGui::CollapsingHeader("Coordinate Axes")) {
        if (coordinateAxes) {
            bool axesVisible = coordinateAxes->isVisible();
            if (ImGui::Checkbox("Show Axes", &axesVisible)) {
                coordinateAxes->setVisible(axesVisible);
                coordinateAxes->regenerateAxes();
                if (renderer) {
                    renderer->setLines(coordinateAxes->getAxisLines());
                }
            }

            if (axesVisible) {
                float axisLength = coordinateAxes->getAxisLength();
                if (ImGui::SliderFloat("Axis Length", &axisLength, 0.5f, 5.0f)) {
                    coordinateAxes->setAxisLength(axisLength);
                    if (renderer) {
                        renderer->setLines(coordinateAxes->getAxisLines());
                    }
                }

                float axisThickness = coordinateAxes->getAxisThickness();
                if (ImGui::SliderFloat("Axis Thickness", &axisThickness, 0.1f, 2.0f)) {
                    coordinateAxes->setAxisThickness(axisThickness);
                    if (renderer) {
                        renderer->setLines(coordinateAxes->getAxisLines());
                    }
                }
            }
        }
    }
    #endif
}

void UI::applyDisplaySettings() {
    // This would apply display settings to the renderer
    // For now, just log the changes
    Utils::logInfo("Display settings changed: Vertices=" + std::to_string(displayVertices) +
                  ", Edges=" + std::to_string(displayEdges) +
                  ", Faces=" + std::to_string(displayFaces));
}

void UI::applyAxesSettings() {
    // Apply axes settings changes
    if (coordinateAxes && renderer) {
        renderer->setLines(coordinateAxes->getAxisLines());
    }
}