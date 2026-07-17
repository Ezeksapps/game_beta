#pragma once

/* BASED ON:
 * https://stackoverflow.com/questions/49609654/quaternion-based-first-person-view-camera
 * https://github.com/Crydsch/camera/blob/main/camera.h
 * https://github.com/hmazhar/moderngl_camera
 */

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define CAMERA_RIGHT        vec3(1.0f, 0.0f, 0.0f)
#define CAMERA_UP           vec3(0.0f, 1.0f, 0.0f)
#define CAMERA_FORWARD      vec3(0.0f, 0.0f, 1.0f) // Camera top-down, treat Z-axis as default forward direction

using namespace glm;

class Camera {

public:

    Camera();
    ~Camera();

    mat4 getViewMatrix();

    /* ---- SETTERS ---- */

    void setPos(const vec3& pos);
    void setTargetPos(const vec3& pos);
    void setOffset(const vec3& offset);
    void rotate(const vec3& angles);

private:

    vec3 forward();
    vec3 up();
    vec3 right();
    vec3 eye();

    quat m_orientation;
    vec3 m_targetPos;
    vec3 m_offset;

    vec3 m_rotationAcc;
};

