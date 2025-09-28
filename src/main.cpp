#include <iostream>
#include "Application.h"
#include "utils/Utils.h"

int main() {
    Utils::logInfo("Starting 3D Model Editor...");

    // Create application instance
    Application app;

    // Initialize application
    if (!app.initialize()) {
        Utils::logError("Failed to initialize application");
        return -1;
    }

    Utils::logInfo("Application initialized successfully");

    // Run main loop
    try {
        app.run();
    } catch (const std::exception& e) {
        Utils::logError("Runtime error: " + std::string(e.what()));
        return -1;
    }

    // Cleanup
    app.shutdown();

    Utils::logInfo("Application shut down successfully");
    return 0;
}