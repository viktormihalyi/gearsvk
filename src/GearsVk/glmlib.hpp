#ifndef GLMLIB_HPP
#define GLMLIB_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


inline std::ostream& operator<< (std::ostream& os, const glm::vec2& vec)
{
    os << "("
       << vec.x << ", "
       << vec.y << ", "
       << ")";
    return os;
}


inline std::ostream& operator<< (std::ostream& os, const glm::vec3& vec)
{
    os << "("
       << vec.x << ", "
       << vec.y << ", "
       << vec.z << ", "
       << ")";
    return os;
}

#endif