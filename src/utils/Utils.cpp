#include "Utils.h"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace Utils {

    void logInfo(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);

        std::cout << "[INFO] "
                  << std::put_time(&tm, "%H:%M:%S")
                  << " " << message << std::endl;
    }

    void logError(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);

        std::cerr << "[ERROR] "
                  << std::put_time(&tm, "%H:%M:%S")
                  << " " << message << std::endl;
    }

    void logWarning(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);

        std::cout << "[WARNING] "
                  << std::put_time(&tm, "%H:%M:%S")
                  << " " << message << std::endl;
    }

    bool fileExists(const std::string& path) {
        std::ifstream file(path);
        return file.good();
    }

    void checkGLError(const char* operation) {
        // TODO Phase 1: Implement OpenGL error checking
        // GLenum error = glGetError();
        // if (error != GL_NO_ERROR) {
        //     logError("OpenGL error in " + std::string(operation) + ": " + std::to_string(error));
        // }

        // For Phase 0, just log that the function was called
        logInfo("OpenGL error check called for: " + std::string(operation) + " (not implemented yet)");
    }

    std::string getFileExtension(const std::string& filename) {
        size_t lastDot = filename.find_last_of(".");
        if (lastDot == std::string::npos) {
            return "";
        }
        return filename.substr(lastDot + 1);
    }

}