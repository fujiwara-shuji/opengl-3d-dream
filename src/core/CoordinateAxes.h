#pragma once

#include "Ray.h"
#include "../math/Vector3.h"
#include <vector>

class CoordinateAxes {
private:
    // Axes lines and origin point
    std::vector<Line> axisLines;
    Vector3 originPoint;

    // Settings
    bool showAxes;
    float axisLength;
    float axisThickness;

    // Axis colors (standard RGB)
    Vector3 xAxisColor;  // Red
    Vector3 yAxisColor;  // Green
    Vector3 zAxisColor;  // Blue

public:
    CoordinateAxes();
    ~CoordinateAxes() = default;

    // Settings
    void setVisible(bool visible) { showAxes = visible; }
    bool isVisible() const { return showAxes; }

    void setAxisLength(float length);
    float getAxisLength() const { return axisLength; }

    void setAxisThickness(float thickness);
    float getAxisThickness() const { return axisThickness; }

    void setAxisColors(const Vector3& xColor, const Vector3& yColor, const Vector3& zColor);

    // Line access
    const std::vector<Line>& getAxisLines() const { return axisLines; }
    const Vector3& getOriginPoint() const { return originPoint; }

    // Regenerate axes based on current settings
    void regenerateAxes();

private:
    void createAxisLines();
};