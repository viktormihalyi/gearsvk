#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Assert.hpp"
#include "Event.hpp"
#include "Frustum.hpp"

#include "glmlib.hpp"


class Camera {
public:
    USING_PTR (Camera);

    glm::vec3 position;
    glm::vec3 ahead;
    glm::vec3 up;
    glm::vec3 right;

    Frustum::U frustum;

    float speed;
    float yaw;
    float pitch;
    float sensitivity;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 viewProjectionMatrix;
    glm::mat4 rayDirMatrix;

    Event<glm::vec3> positionChanged;

public:
    enum class MovementDirection {
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down
    };

    Camera (const glm::vec3& position, const glm::vec3& ahead, float aspect);

    const glm::vec3& GetPosition () const { return position; }
    const glm::mat4& GetRayDirMatrix () const { return rayDirMatrix; }

    void SetAspectRatio (float value);

    void SetViewMatrix (const glm::mat4& matrix) { viewMatrix = matrix; }
    void SetProjectionMatrix (const glm::mat4& matrix) { projectionMatrix = matrix; }
    void UpdateViewProjectionMatrix ();

    const glm::mat4& GetViewMatrix () const { return viewMatrix; }
    const glm::mat4& GetProjectionMatrix () const { return projectionMatrix; }
    const glm::mat4& GetViewProjectionMatrix () const { return viewProjectionMatrix; }

    void Move (MovementDirection, float dt);
    void UpdateVectors ();
    void ProcessMouseInput (const glm::vec2& delta);

    float GetSpeed () const { return speed; }
    void  SetSpeed (float value) { speed = value; }

    float GetBackPlane () const;
    float GetFrontPlane () const;
    void  SetFrontAndBackPlane (float back, float front);
};


inline void Camera::SetAspectRatio (float value)
{
    PerspectiveFrustum* f = dynamic_cast<PerspectiveFrustum*> (frustum.get ());
    if (ERROR (f == nullptr)) {
        return;
    }

    f->SetAspectRatio (value);
}


inline float Camera::GetBackPlane () const
{
    PerspectiveFrustum* f = dynamic_cast<PerspectiveFrustum*> (frustum.get ());
    if (ERROR (f == nullptr)) {
        return 0.f;
    }

    return f->GetBackPlane ();
}


inline float Camera::GetFrontPlane () const
{
    PerspectiveFrustum* f = dynamic_cast<PerspectiveFrustum*> (frustum.get ());
    if (ERROR (f == nullptr)) {
        return 0.f;
    }

    return f->GetFrontPlane ();
}


inline void Camera::SetFrontAndBackPlane (float front, float back)
{
    PerspectiveFrustum* f = dynamic_cast<PerspectiveFrustum*> (frustum.get ());
    if (ERROR (f == nullptr)) {
        return;
    }

    f->SetFrontPlane (front);
    f->SetBackPlane (back);

    UpdateViewProjectionMatrix ();
}

#endif
