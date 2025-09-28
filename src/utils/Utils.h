#pragma once

#include <iostream>
#include <string>
#include <fstream>

namespace Utils {
    // Basic assertion macro
    #define ASSERT(condition, message) \
        if (!(condition)) { \
            std::cerr << "ASSERTION FAILED: " << message << std::endl; \
            std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << std::endl; \
            exit(1); \
        }

    // Logging functions
    void logInfo(const std::string& message);
    void logError(const std::string& message);
    void logWarning(const std::string& message);

    // File utilities
    bool fileExists(const std::string& path);

    // OpenGL error checking (Phase 1)
    void checkGLError(const char* operation);

    // String utilities
    std::string getFileExtension(const std::string& filename);

    // Math utilities (basic)
    constexpr float PI = 3.14159265359f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    // Clamp function
    template<typename T>
    T clamp(T value, T min, T max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
}