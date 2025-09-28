#include "Vector3.h"
#include "Matrix4.h"
#include "../utils/Utils.h"
#include <iostream>
#include <iomanip>

void testVector3() {
    Utils::logInfo("Testing Vector3 class...");

    // Basic operations
    Vector3 v1(1.0f, 2.0f, 3.0f);
    Vector3 v2(4.0f, 5.0f, 6.0f);

    std::cout << "v1 = " << v1 << std::endl;
    std::cout << "v2 = " << v2 << std::endl;

    Vector3 sum = v1 + v2;
    std::cout << "v1 + v2 = " << sum << std::endl;

    Vector3 diff = v2 - v1;
    std::cout << "v2 - v1 = " << diff << std::endl;

    Vector3 scaled = v1 * 2.0f;
    std::cout << "v1 * 2 = " << scaled << std::endl;

    // Dot product
    float dot = Vector3::dot(v1, v2);
    std::cout << "dot(v1, v2) = " << dot << std::endl;

    // Cross product
    Vector3 cross = Vector3::cross(v1, v2);
    std::cout << "cross(v1, v2) = " << cross << std::endl;

    // Length and normalization
    std::cout << "v1.length() = " << v1.length() << std::endl;
    Vector3 normalized = v1.normalized();
    std::cout << "v1.normalized() = " << normalized << std::endl;
    std::cout << "normalized.length() = " << normalized.length() << std::endl;

    // Predefined vectors
    std::cout << "Vector3::UP = " << Vector3::UP << std::endl;
    std::cout << "Vector3::RIGHT = " << Vector3::RIGHT << std::endl;
    std::cout << "Vector3::FORWARD = " << Vector3::FORWARD << std::endl;

    Utils::logInfo("Vector3 tests completed");
}

void testMatrix4() {
    Utils::logInfo("Testing Matrix4 class...");

    // Identity matrix
    Matrix4 identity = Matrix4::identity();
    std::cout << "Identity matrix:" << std::endl;
    identity.print();
    std::cout << std::endl;

    // Translation matrix
    Matrix4 translation = Matrix4::translation(Vector3(1.0f, 2.0f, 3.0f));
    std::cout << "Translation matrix (1, 2, 3):" << std::endl;
    translation.print();
    std::cout << std::endl;

    // Rotation matrices
    Matrix4 rotX = Matrix4::rotationX(Utils::PI / 4.0f);  // 45 degrees
    std::cout << "Rotation X (45 degrees):" << std::endl;
    rotX.print();
    std::cout << std::endl;

    Matrix4 rotY = Matrix4::rotationY(Utils::PI / 4.0f);
    std::cout << "Rotation Y (45 degrees):" << std::endl;
    rotY.print();
    std::cout << std::endl;

    Matrix4 rotZ = Matrix4::rotationZ(Utils::PI / 4.0f);
    std::cout << "Rotation Z (45 degrees):" << std::endl;
    rotZ.print();
    std::cout << std::endl;

    // Scale matrix
    Matrix4 scale = Matrix4::scale(Vector3(2.0f, 3.0f, 4.0f));
    std::cout << "Scale matrix (2, 3, 4):" << std::endl;
    scale.print();
    std::cout << std::endl;

    // Matrix multiplication
    Matrix4 combined = translation * rotZ * scale;
    std::cout << "Combined transformation (translate * rotateZ * scale):" << std::endl;
    combined.print();
    std::cout << std::endl;

    // Vector transformation
    Vector3 point(1.0f, 0.0f, 0.0f);
    Vector3 transformed = combined.transformPoint(point);
    std::cout << "Transform point " << point << " -> " << transformed << std::endl;

    // Matrix inverse
    Matrix4 inv = combined.inverse();
    std::cout << "Inverse of combined matrix:" << std::endl;
    inv.print();
    std::cout << std::endl;

    // Verify inverse
    Matrix4 shouldBeIdentity = combined * inv;
    std::cout << "combined * inverse (should be identity):" << std::endl;
    shouldBeIdentity.print();
    std::cout << std::endl;

    // Perspective matrix
    Matrix4 perspective = Matrix4::perspective(Utils::PI / 3.0f, 16.0f/9.0f, 0.1f, 100.0f);
    std::cout << "Perspective matrix (60 deg FOV, 16:9 aspect, near=0.1, far=100):" << std::endl;
    perspective.print();
    std::cout << std::endl;

    // LookAt matrix
    Matrix4 view = Matrix4::lookAt(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 0, 1));
    std::cout << "LookAt matrix (eye=(0,0,5), target=(0,0,0), up=(0,0,1)):" << std::endl;
    view.print();
    std::cout << std::endl;

    Utils::logInfo("Matrix4 tests completed");
}

void testMathOperations() {
    Utils::logInfo("Testing advanced math operations...");

    // Test reflection calculation
    Vector3 incident(1.0f, -1.0f, 0.0f);
    Vector3 normal(0.0f, 1.0f, 0.0f);
    Vector3 reflected = Vector3::reflect(incident, normal);

    std::cout << "Incident: " << incident << std::endl;
    std::cout << "Normal: " << normal << std::endl;
    std::cout << "Reflected: " << reflected << std::endl;

    // Test linear interpolation
    Vector3 start(0.0f, 0.0f, 0.0f);
    Vector3 end(10.0f, 10.0f, 10.0f);
    Vector3 midpoint = Vector3::lerp(start, end, 0.5f);

    std::cout << "Lerp from " << start << " to " << end << " at t=0.5: " << midpoint << std::endl;

    // Test coordinate system
    Vector3 right = Vector3::RIGHT;
    Vector3 forward = Vector3::FORWARD;
    Vector3 up = Vector3::UP;

    Vector3 crossProduct = Vector3::cross(right, forward);
    std::cout << "Right x Forward = " << crossProduct << " (should be Up = " << up << ")" << std::endl;

    Utils::logInfo("Advanced math operations tests completed");
}

int main() {
    Utils::logInfo("Starting Math Library Tests");
    std::cout << std::fixed << std::setprecision(3);

    try {
        testVector3();
        std::cout << "\n" << std::string(50, '-') << "\n" << std::endl;

        testMatrix4();
        std::cout << "\n" << std::string(50, '-') << "\n" << std::endl;

        testMathOperations();

    } catch (const std::exception& e) {
        Utils::logError("Test failed with exception: " + std::string(e.what()));
        return -1;
    }

    Utils::logInfo("All math library tests completed successfully!");
    return 0;
}