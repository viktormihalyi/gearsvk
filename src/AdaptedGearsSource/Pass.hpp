#ifndef PASS_HPP
#define PASS_HPP

#include "Utils.hpp"

class Pass : public Noncopyable {
public:
    USING_PTR (Pass);

    std::string name;
    std::string briefDescription;

    uint32_t startingFrame;
    uint32_t duration;

    // TODO shader attributes, uniforms, ...
};

#endif