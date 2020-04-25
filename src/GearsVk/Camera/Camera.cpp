#include "Camera.hpp"

#include <algorithm>

#include "glmlib.hpp"


constexpr glm::vec3 WORLD_UP (0, 0, 1);


Camera::Camera (const glm::vec3& position,
                const glm::vec3& ahead,
                float            aspect)
    : position (position)
    , ahead (ahead)
    , frustum (new PerspectiveFrustum (100.f, 0.001f, 120.f, aspect))
    , speed (1.f)
    , yaw (90.f)
    , pitch (0)
    , sensitivity (0.2f)
    , viewMatrix ([&] () { return glm::lookAt (this->position, this->position + this->ahead, up); })
    , projectionMatrix ([&] () { return frustum->GetMatrix (); })
    , viewProjectionMatrix ([&] () { return projectionMatrix.Get () * viewMatrix.Get (); })
    , rayDirMatrix ([&] () { return glm::inverse (projectionMatrix.Get () * glm::translate (glm::mat4 (1.0), this->position)); })
{
    UpdateVectors ();
}


void Camera::UpdateVectors ()
{
    using std::cos, std::sin;

    ahead.x = cos (glm::radians (pitch)) * cos (glm::radians (yaw));
    ahead.y = cos (glm::radians (pitch)) * sin (glm::radians (yaw));
    ahead.z = sin (glm::radians (pitch));
    ahead   = glm::normalize (ahead);

    right = glm::normalize (glm::cross (ahead, WORLD_UP));
    up    = glm::normalize (glm::cross (right, ahead));

    viewMatrix.Invalidate ();
    viewProjectionMatrix.Invalidate ();
}


void Camera::Move (Camera::MovementDirection dir, float dt)
{
    const float speedDt = speed * dt;

    switch (dir) {
        case MovementDirection::Forward:
            position += ahead * speedDt;
            break;
        case MovementDirection::Backward:
            position -= ahead * speedDt;
            break;
        case MovementDirection::Left:
            position -= right * speedDt;
            break;
        case MovementDirection::Right:
            position += right * speedDt;
            break;
        case MovementDirection::Up:
            position += WORLD_UP * speedDt;
            break;
        case MovementDirection::Down:
            position -= WORLD_UP * speedDt;
            break;

        default:
            ASSERT (false);
            break;
    }

    viewMatrix.Invalidate ();
    rayDirMatrix.Invalidate ();
    viewProjectionMatrix.Invalidate ();

    positionChanged (position);
}


void Camera::ProcessMouseInput (const glm::vec2& delta)
{
    yaw += delta.x * sensitivity;
    pitch -= delta.y * sensitivity;

    pitch = std::clamp (pitch, -89.f, 89.f);

    UpdateVectors ();
}
