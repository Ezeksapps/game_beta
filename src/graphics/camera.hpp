#pragma once

/* Adapted from https://github.com/hmazhar/moderngl_camera/blob/master/camera.hpp
 * ---------
 * CHANGES:
 * ---------
 * Renamed any reference to a 'heading' to yaw
 * Changed functions to Pascal to Camel case
 * Changed member variables to m_<varname> format
 * Removed ortho camera mode, as it isn't needed
 * Removed unused or unecessary functions
 * Removed any reference to any matrix that isn't the view matrix (this should not handle or know about proj/model matrices)
 * Corrected so that angles are all considered in radians
 * Changed euler angle setters to one unified rotate func (but doesn't consider roll yet)
 * Changed to store full orientation quaternion
 * Removed unecessary movement damping in view matrix calculation
 * Changed so camera's up and right vectors are declared as macros
 * Changed to a Z-up coordinate system
 * Removed yaw and pitch rate limiters
 *
 * NOTE: Some ideas (such as orientation quat, unified rotate and accumulator) were carried over from Crydsch camera
*/


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define CAMERA_RIGHT    glm::vec3(1.0f, 0.0f, 0.0f)
#define CAMERA_FORWARD  glm::vec3(0.0f, 1.0f, 0.0f)
#define CAMERA_UP       glm::vec3(0.0f, 0.0f, 1.0f)

enum CameraDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    FORWARD,
    BACK
};

class Camera {

public:
    Camera();
    ~Camera();

    glm::mat4 getViewMatrix();

    // Given a specific moving direction, the camera will be moved in the appropriate direction
    // For a spherical camera this will be around the look_at point
    // For a free camera a delta will be computed for the direction of movement.
    void Move(CameraDirection dir);

    void rotate(const glm::vec3& angles);

    /* ---- SETTERS ---- */

    void setPos(const glm::vec3& pos);
    void setTargetPos(const glm::vec3& pos);


    float m_scale;
    float m_yaw;
    float m_pitch;

    bool  m_moveCamera;

    glm::quat m_orientation;

    glm::vec3 m_rotationAcc;

    glm::vec3 m_pos;
    glm::vec3 m_deltaPos;
    glm::vec3 m_targetPos;
    glm::vec3 m_direction;

};

