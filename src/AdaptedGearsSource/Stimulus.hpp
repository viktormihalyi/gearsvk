#ifndef STIMULUS_HPP
#define STIMULUS_HPP

#include "Utils.hpp"

#include "AdaptedGearsSource/Pass.hpp"

class Stimulus : public Noncopyable {
private:
    std::string name;
    std::string brief;
    uint32_t    startingFrame;
    uint32_t    duration;

    std::vector<Pass::P> passes;

public:
    USING_PTR (Stimulus);
    Stimulus () {}
};

#endif