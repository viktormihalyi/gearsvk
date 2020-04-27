#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Assert.hpp"
#include "Cache.hpp"
#include "Event.hpp"
#include "Frustum.hpp"
#include "Persistent.hpp"

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

    // TODO test if this cached stuff is slower or not

    Cache<glm::mat4> viewMatrix;
    Cache<glm::mat4> projectionMatrix;
    Cache<glm::mat4> viewProjectionMatrix;
    Cache<glm::mat4> rayDirMatrix;

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
    const glm::mat4& GetRayDirMatrix () { return rayDirMatrix; }

    void SetAspectRatio (float value);


    const glm::mat4& GetViewMatrix () { return viewMatrix; }
    const glm::mat4& GetProjectionMatrix () { return projectionMatrix; }
    const glm::mat4& GetViewProjectionMatrix () { return viewProjectionMatrix; }

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

    viewMatrix.Invalidate ();
    projectionMatrix.Invalidate ();
    rayDirMatrix.Invalidate ();
    viewProjectionMatrix.Invalidate ();
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

    viewMatrix.Invalidate ();
    projectionMatrix.Invalidate ();
    rayDirMatrix.Invalidate ();
    viewProjectionMatrix.Invalidate ();
}

#endif
