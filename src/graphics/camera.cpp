#include "camera.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"

Camera::Camera() {}

Camera::~Camera() {}

glm::vec3 Camera::forward() {
    return m_orientation * CAMERA_FORWARD;
}

glm::vec3 Camera::up() {
    return m_orientation * CAMERA_UP;
}

glm::vec3 Camera::right() {
    return m_orientation * CAMERA_RIGHT;
}

glm::vec3 Camera::eye() {
    return m_targetPos + m_orientation * m_offset;
}

glm::mat4 Camera::getViewMatrix() {

    quat pitchQuat = angleAxis(m_rotationAcc.x, right());
    quat yawQuat = angleAxis(m_rotationAcc.y, CAMERA_UP);
    glm::quat rollQuat = glm::angleAxis(m_rotationAcc.z, CAMERA_FORWARD);

    m_rotationAcc = glm::vec3(0.0f, 0.0f, 0.0f); // reset accumulator

    /* Get rotation matrix from quaternion */
    m_orientation = normalize(yawQuat * pitchQuat * rollQuat * m_orientation);
    mat4 rotationMatrix = mat4_cast(m_orientation);

    /* Get translation matrix */
    mat4 translationMatrix = mat4(1.0f);
    translationMatrix = translate(translationMatrix, -eye());

    return rotationMatrix * translationMatrix;
}

void Camera::setOffset(const glm::vec3& offset) {
    m_offset = offset;
}

void Camera::rotate(const glm::vec3& angles) {
    m_rotationAcc.x += angles.x;
    m_rotationAcc.y += angles.y;
    m_rotationAcc.z += angles.z;
}

void Camera::setTargetPos(const glm::vec3& pos) {
    m_targetPos = pos;
}
 
