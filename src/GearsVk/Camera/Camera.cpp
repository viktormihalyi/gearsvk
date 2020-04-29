#include "Camera.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "glmlib.hpp"


class GlobalUpVectorProvider {
public:
    USING_PTR_ABSTRACT (GlobalUpVectorProvider);

    virtual ~GlobalUpVectorProvider () = default;

    virtual void             ProcessMouseInput (Camera& camera, const glm::vec2& delta) const = 0;
    virtual glm::vec3        GetAheadVector (float pitch, float yaw) const                    = 0;
    virtual const glm::vec3& GetUpVector () const                                             = 0;
};


class GlobalZ final : public GlobalUpVectorProvider {
private:
    const glm::vec3 upVector;

public:
    USING_PTR (GlobalZ);
    GlobalZ ()
        : upVector (0, 0, 1)
    {
    }

    virtual void ProcessMouseInput (Camera& camera, const glm::vec2& delta) const override
    {
        camera.yaw -= delta.x * camera.sensitivity;
        camera.pitch += delta.y * camera.sensitivity;
        camera.pitch = std::clamp (camera.pitch, -89.f, 89.f);
    }

    virtual glm::vec3 GetAheadVector (float pitch, float yaw) const override
    {
        using std::cos, std::sin;

        glm::vec3 ahead;
        ahead.x = cos (glm::radians (pitch)) * cos (glm::radians (yaw));
        ahead.y = cos (glm::radians (pitch)) * sin (glm::radians (yaw));
        ahead.z = sin (glm::radians (pitch));
        return glm::normalize (ahead);
    }

    virtual const glm::vec3& GetUpVector () const override
    {
        return upVector;
    }
};

// TODO
class GlobalY final : public GlobalUpVectorProvider {
private:
    const glm::vec3 upVector;

public:
    USING_PTR (GlobalY);
    GlobalY ()
        : upVector (0, 1, 0)
    {
    }

    virtual void ProcessMouseInput (Camera& camera, const glm::vec2& delta) const override
    {
        camera.yaw += delta.x * camera.sensitivity;
        camera.pitch += delta.y * camera.sensitivity;
        camera.pitch = std::clamp (camera.pitch, -89.f, 89.f);
    }

    virtual glm::vec3 GetAheadVector (float pitch, float yaw) const override
    {
        using std::cos, std::sin;

        glm::vec3 ahead;
        ahead.x = cos (glm::radians (pitch)) * cos (glm::radians (yaw));
        ahead.z = cos (glm::radians (pitch)) * sin (glm::radians (yaw));
        ahead.y = sin (glm::radians (pitch));
        return glm::normalize (ahead);
    }

    virtual const glm::vec3& GetUpVector () const override
    {
        return upVector;
    }
};


const GlobalZ upVector;


// TODO calulcate initial pitch and yaw from ahead vector

Camera::Camera (const glm::vec3& position,
                const glm::vec3& ahead,
                float            aspect)
    : position (position)
    , ahead (ahead)
    , frustum (new PerspectiveFrustum (100.f, 0.001f, 80.f, aspect))
    , speed (1.f)
    , yaw (45.f)
    , pitch (0)
    , sensitivity (0.2f)
    , viewMatrix ([&] () { return glm::lookAt (this->position, this->position + this->ahead, up); })
    , projectionMatrix ([&] () { return frustum->GetMatrix (); })
    , viewProjectionMatrix ([&] () { return projectionMatrix.Get () * viewMatrix.Get (); })
    , rayDirMatrix ([&] () { return glm::inverse (viewProjectionMatrix.Get () * glm::translate (glm::mat4 (1.f), this->position)); })
{
    UpdateVectors ();

    //positionChanged += [&] (glm::vec3) {
    //    std::cout << "position: " << this->position << std::endl;
    //    std::cout << "ahead:    " << this->ahead << std::endl;
    //};
}


void Camera::UpdateVectors ()
{
    ahead = upVector.GetAheadVector (pitch, yaw);

    right = glm::normalize (glm::cross (upVector.GetUpVector (), ahead));
    up    = glm::normalize (glm::cross (right, ahead));

    viewMatrix.Invalidate ();
    projectionMatrix.Invalidate ();
    rayDirMatrix.Invalidate ();
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
            position += upVector.GetUpVector () * speedDt;
            break;
        case MovementDirection::Down:
            position -= upVector.GetUpVector () * speedDt;
            break;

        default:
            ASSERT (false);
            break;
    }

    viewMatrix.Invalidate ();
    projectionMatrix.Invalidate ();
    rayDirMatrix.Invalidate ();
    viewProjectionMatrix.Invalidate ();

    positionChanged (position);
}


void Camera::ProcessMouseInput (const glm::vec2& delta)
{
    upVector.ProcessMouseInput (*this, delta);

    UpdateVectors ();
}
