#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include "Ptr.hpp"

#include "glmlib.hpp"


class Frustum {
public:
    USING_PTR_ABSTRACT (Frustum);

    virtual ~Frustum () = default;

    virtual glm::mat4 GetMatrix () const = 0;
};


class PerspectiveFrustum : public Frustum {
private:
    float backPlane;
    float frontPlane;
    float fov;
    float aspectRatio;

public:
    PerspectiveFrustum (float backPlane, float frontPlane, float fov, float aspectRatio);

    void SetAspectRatio (float value) { aspectRatio = value; }
    void SetBackPlane (float value) { backPlane = value; }
    void SetFrontPlane (float value) { frontPlane = value; }

    float GetBackPlane () const { return backPlane; }
    float GetFrontPlane () const { return frontPlane; }

    glm::mat4 GetMatrix () const override;
};


class OrthographicFrustum : public Frustum {
private:
    float left;
    float right;
    float bottom;
    float top;

public:
    OrthographicFrustum (float left, float right, float bottom, float top);

    glm::mat4 GetMatrix () const override;
};

#endif