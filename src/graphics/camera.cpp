#include "camera.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include <iostream>

/* FIXED ERRORS IN CALCS BASED ON SOURCES:
 * https://stackoverflow.com/questions/49609654/quaternion-based-first-person-view-camera
 */


// orientation is quat used to calc view matrix
// distance and cam pos must now be considered

Camera::Camera() {
    m_direction = glm::vec3(0.0f, 0.0f, 0.0f);
    m_orientation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    m_deltaPos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_scale = 0.5f;
    m_moveCamera = false;
}

Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix() {
    std::cout << "rotation accumulator:\n[ " << m_rotationAcc[0] << ", " << m_rotationAcc[1] << ", " << m_rotationAcc[2] << " ]" << std::endl;

    // direction and eye are equivalent terms

    m_direction = glm::normalize(m_targetPos - m_pos);
   // m_direction = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f));

    std::cout << "Normalised direction vector:\n[ " << m_direction[0] << ", " << m_direction[1] << ", " << m_direction[2] << " ]" << std::endl;

    // NOTE: Whether the coord system is Y-up or Z-up, the cross of cam direction and CAMERA_UP is always the pitch axis
    // TODO: Update to allow adjustments to roll (though I never actually use it, but will be helpful for debug view)

    std::cout << "Executing view matrix gen\n";

    glm::quat pitchQuat = glm::angleAxis(m_rotationAcc.x, CAMERA_RIGHT);
    glm::quat yawQuat = glm::angleAxis(m_rotationAcc.y, CAMERA_FORWARD);
    glm::quat rollQuat = glm::angleAxis(m_rotationAcc.z, CAMERA_UP); // ROLL CURRENTLY UNUSED

    std::cout << "Pitch quaternion\n-=-=-=-=-=-=-=-=-\n"
    << "[ "<< pitchQuat[0] << ", " << pitchQuat[1] << ", " << pitchQuat[2] << ", " << pitchQuat[3] << " ]" << std::endl;

    std::cout << "Yaw quaternion\n-=-=-=-=-=-=-=-=-\n"
    << "[ "<< yawQuat[0] << ", " << yawQuat[1] << ", " << yawQuat[2] << ", " << yawQuat[3] << " ]" << std::endl;

    m_rotationAcc = glm::vec3(0.0f, 0.0f, 0.0f); // reset accumulator


    //direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    //direction.y = sin(glm::radians(pitch));
    //direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    //cameraFront = glm::normalize(direction);

   // where eye = return cm_add(_cam->target_position, cm_scale(camera_forward(_cam), -_cam->target_distance));

    /* Get rotation matrix from quaternion */
   m_orientation = glm::normalize(pitchQuat * yawQuat);
   glm::mat4 rotate = glm::mat4_cast(m_orientation);

   /* Get translation matrix */
   glm::mat4 translate = glm::mat4(1.0f);
   translate = glm::translate(translate, -m_direction);

   return glm::mat4(rotate * translate);
}

void Camera::rotate(const glm::vec3& angles) {
    m_rotationAcc.x += angles.x;
    m_rotationAcc.y += angles.y;
    m_rotationAcc.z += angles.z;
}

void Camera::setPos(const glm::vec3& pos) {
    m_pos = pos;
}

void Camera::setTargetPos(const glm::vec3& pos) {
    m_targetPos = pos;

    std::cout << "Set target pos:\n[ " << m_targetPos[0] << ", " << m_targetPos[1] << ", " << m_targetPos[2] << " ]" << std::endl;
}


/* This is silly, change so that this takes vec3 of translation vec */
/*void Camera::Move(CameraDirection dir) {
    switch (dir) {
        case UP:
            m_deltaPos += CAMERA_UP * m_scale;
            break;
        case DOWN:
            m_deltaPos -= CAMERA_UP * m_scale;
            break;
        case LEFT:
            m_deltaPos -= glm::cross(m_direction, CAMERA_UP) * m_scale;
            break;
        case RIGHT:
            m_deltaPos += glm::cross(m_direction, CAMERA_UP) * m_scale;
            break;
        case FORWARD:
            m_deltaPos += m_direction * m_scale;
            break;
        case BACK:
            m_deltaPos -= m_direction * m_scale;
            break;
    }
}*/

