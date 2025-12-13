#include "CvUtils.hpp"

#include <glm/ext/matrix_transform.hpp>

glm::mat4 cvToEngineRotation() {
    // Convert CV camera (Z up, X forward)
    // to engine camera (Y up, -Z forward)
    glm::mat4 R(1.0f);

    // Rotate +90° around X: Z-up → Y-up
    R = glm::rotate(R, glm::radians(90.0f), glm::vec3(1, 0, 0));

    return R;
}