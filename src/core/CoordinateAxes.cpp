#include "CoordinateAxes.h"
#include "../utils/Utils.h"

CoordinateAxes::CoordinateAxes()
    : showAxes(true)
    , axisLength(2.0f)
    , axisThickness(1.0f)
    , xAxisColor(1.0f, 0.0f, 0.0f)  // Red
    , yAxisColor(0.0f, 1.0f, 0.0f)  // Green
    , zAxisColor(0.0f, 0.0f, 1.0f)  // Blue
{
    createAxisLines();
}

void CoordinateAxes::setAxisLength(float length) {
    if (length > 0.0f) {
        axisLength = length;
        regenerateAxes();
        Utils::logInfo("Axis length set to: " + std::to_string(length));
    }
}

void CoordinateAxes::setAxisThickness(float thickness) {
    if (thickness > 0.0f) {
        axisThickness = thickness;
        regenerateAxes();
        Utils::logInfo("Axis thickness set to: " + std::to_string(thickness));
    }
}

void CoordinateAxes::setAxisColors(const Vector3& xColor, const Vector3& yColor, const Vector3& zColor) {
    xAxisColor = xColor;
    yAxisColor = yColor;
    zAxisColor = zColor;
    regenerateAxes();
    Utils::logInfo("Axis colors updated");
}

void CoordinateAxes::regenerateAxes() {
    if (showAxes) {
        createAxisLines();
    } else {
        axisLines.clear();
    }
}

void CoordinateAxes::createAxisLines() {
    axisLines.clear();

    if (!showAxes) {
        return;
    }

    Vector3 origin(0, 0, 0);

    // X-axis (origin to +X direction) - Red
    Vector3 xEnd(axisLength, 0, 0);
    axisLines.emplace_back(origin, xEnd, xAxisColor, axisThickness);

    // Y-axis (origin to +Y direction) - Green
    Vector3 yEnd(0, axisLength, 0);
    axisLines.emplace_back(origin, yEnd, yAxisColor, axisThickness);

    // Z-axis (origin to +Z direction) - Blue
    Vector3 zEnd(0, 0, axisLength);
    axisLines.emplace_back(origin, zEnd, zAxisColor, axisThickness);

    Utils::logInfo("Created coordinate axes: 3 lines, length=" + std::to_string(axisLength));
}