#!/bin/bash

# 3D Model Editor Compilation Script
# Compiles all phases of the 3D model editor project

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
echo "  test-phase0  - Basic project structure test"
echo "  math-test    - Math library test (Vector3, Matrix4)"
echo "  test-phase1  - Rendering foundation test"
echo "  render-test  - Complex rendering test"
echo "  model-editor - Main 3D model editor application"
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
    "test-phase0")
        if [ -f "src/test/Phase0Test.cpp" ]; then
            compile_target "test-phase0" "src/test/Phase0Test.cpp"
        else
            echo -e "${RED}✗ Phase0Test.cpp not found${NC}"
            exit 1
        fi
        ;;
    "math-test")
        if [ -f "src/test/MathTest.cpp" ]; then
            compile_target "math-test" "src/test/MathTest.cpp"
        else
            echo -e "${RED}✗ MathTest.cpp not found${NC}"
            exit 1
        fi
        ;;
    "test-phase1")
        if [ -f "src/test/Phase1Test.cpp" ]; then
            compile_target "test-phase1" "src/test/Phase1Test.cpp"
        else
            echo -e "${RED}✗ Phase1Test.cpp not found${NC}"
            exit 1
        fi
        ;;
    "render-test")
        if [ -f "src/rendering/ComplexRenderTest.cpp" ]; then
            compile_target "render-test" "src/rendering/ComplexRenderTest.cpp"
        else
            echo -e "${RED}✗ ComplexRenderTest.cpp not found${NC}"
            exit 1
        fi
        ;;
    "model-editor"|"main"|"test-phase5")
        if [ -f "src/Main.cpp" ]; then
            compile_target "model-editor" "src/Main.cpp"
        else
            echo -e "${RED}✗ Main.cpp not found${NC}"
            exit 1
        fi
        ;;
    "all")
        echo -e "${YELLOW}Compiling all available targets...${NC}"
        SUCCESS_COUNT=0
        TOTAL_COUNT=0

        for test_file in src/test/Phase0Test.cpp src/test/MathTest.cpp src/test/Phase1Test.cpp src/Main.cpp src/rendering/ComplexRenderTest.cpp; do
            if [ -f "$test_file" ]; then
                TOTAL_COUNT=$((TOTAL_COUNT + 1))
                basename_file=$(basename "$test_file" .cpp)
                case $basename_file in
                    "Phase0Test") target_name="test-phase0" ;;
                    "MathTest") target_name="math-test" ;;
                    "Phase1Test") target_name="test-phase1" ;;
                    "Main") target_name="model-editor" ;;
                    "ComplexRenderTest") target_name="render-test" ;;
                esac

                if compile_target "$target_name" "$test_file"; then
                    SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
                fi
            fi
        done

        echo -e "${BLUE}Compilation summary: $SUCCESS_COUNT/$TOTAL_COUNT targets successful${NC}"
        if [ $SUCCESS_COUNT -eq $TOTAL_COUNT ]; then
            echo -e "${GREEN}✓ All targets compiled successfully!${NC}"
        else
            echo -e "${YELLOW}⚠ Some targets failed to compile${NC}"
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
        echo "  test-phase0  - Basic project structure test"
        echo "  math-test    - Math library test"
        echo "  test-phase1  - Rendering foundation test"
        echo "  render-test  - Complex rendering test"
        echo "  model-editor - Main 3D model editor application (default)"
        echo "  main         - Alias for model-editor"
        echo "  all          - Compile all available targets"
        echo "  clean        - Clean build directory"
        echo "  help         - Show this help message"
        echo
        echo "Examples:"
        echo "  $0                    # Compile main application"
        echo "  $0 model-editor       # Compile 3D model editor"
        echo "  $0 all                # Compile all targets"
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