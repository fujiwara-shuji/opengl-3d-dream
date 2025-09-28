#include "Vector3.h"

// Define static constants
const Vector3 Vector3::ZERO(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UNIT_X(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UNIT_Y(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UNIT_Z(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::UP(0.0f, 0.0f, 1.0f);        // Z-up coordinate system
const Vector3 Vector3::RIGHT(1.0f, 0.0f, 0.0f);     // X-right
const Vector3 Vector3::FORWARD(0.0f, 1.0f, 0.0f);   // Y-forward (into screen)