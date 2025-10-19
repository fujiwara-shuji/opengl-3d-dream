#!/bin/bash

# 3D Model Editor Compilation Script
# Compiles the 3D model editor application

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== 3D Model Editor Compilation Script ===${NC}"

# Create build directory if it doesn't exist
mkdir -p build

# Compiler settings
CXX="g++"
CXXFLAGS="-std=c++17 -I./src -I./external/imgui -I./external/imgui/backends -pthread -O2 -DIMGUI_AVAILABLE"
LIBS="-lglfw -lGL -lm"

# Source files (common to all targets)
COMMON_SOURCES="
src/math/Vector3.cpp
src/math/Matrix4.cpp
src/core/Model.cpp
src/core/Camera.cpp
src/core/CoordinateAxes.cpp
src/core/RayIntersection.cpp
src/rendering/SoftwareRenderer.cpp
src/input/InputHandler.cpp
src/ui/UI.cpp
src/utils/Utils.cpp
external/imgui/imgui.cpp
external/imgui/imgui_demo.cpp
external/imgui/imgui_draw.cpp
external/imgui/imgui_tables.cpp
external/imgui/imgui_widgets.cpp
external/imgui/backends/imgui_impl_glfw.cpp
external/imgui/backends/imgui_impl_opengl2.cpp
"

# Function to compile a target
compile_target() {
    local target_name=$1
    local main_file=$2
    local output_file="build/$target_name"

    echo -e "${YELLOW}Compiling $target_name...${NC}"

    if $CXX $CXXFLAGS -o $output_file $main_file $COMMON_SOURCES $LIBS; then
        echo -e "${GREEN}✓ $target_name compiled successfully${NC}"
        return 0
    else
        echo -e "${RED}✗ Failed to compile $target_name${NC}"
        return 1
    fi
}

# Compilation targets
echo -e "${BLUE}Available compilation targets:${NC}"
echo "  model-editor - Main 3D model editor application (default)"
echo "  clean        - Clean build directory"
echo

# Check command line argument
if [ $# -eq 0 ]; then
    echo -e "${YELLOW}No target specified. Compiling main application (model-editor)...${NC}"
    TARGET="model-editor"
else
    TARGET=$1
fi

# Compile based on target
case $TARGET in
    "model-editor"|"main")
        if [ -f "src/Main.cpp" ]; then
            compile_target "model-editor" "src/Main.cpp"
        else
            echo -e "${RED}✗ Main.cpp not found${NC}"
            exit 1
        fi
        ;;
    "clean")
        echo -e "${YELLOW}Cleaning build directory...${NC}"
        rm -rf build/*
        echo -e "${GREEN}✓ Build directory cleaned${NC}"
        ;;
    "help"|"-h"|"--help")
        echo -e "${BLUE}Usage: $0 [target]${NC}"
        echo
        echo "Available targets:"
        echo "  model-editor - Main 3D model editor application (default)"
        echo "  main         - Alias for model-editor"
        echo "  clean        - Clean build directory"
        echo "  help         - Show this help message"
        echo
        echo "Examples:"
        echo "  $0                    # Compile main application"
        echo "  $0 model-editor       # Compile 3D model editor"
        echo "  $0 clean              # Clean build directory"
        ;;
    *)
        echo -e "${RED}✗ Unknown target: $TARGET${NC}"
        echo -e "${YELLOW}Use '$0 help' to see available targets${NC}"
        exit 1
        ;;
esac

echo -e "${BLUE}=== Compilation Complete ===${NC}"

# Show available executables
if [ -d "build" ] && [ "$(ls -A build 2>/dev/null)" ]; then
    echo -e "${BLUE}Available executables in build/:${NC}"
    ls -la build/ | grep -E '^-.*x.*' | awk '{print "  " $9}' || echo "  (none)"
    echo
    echo -e "${YELLOW}Run with: ./build/[executable_name]${NC}"
fi