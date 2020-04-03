#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include "Utils.hpp"

#include "AdaptedGearsSource/Response.hpp"
#include "AdaptedGearsSource/Stimulus.hpp"

class Sequence : public Noncopyable {
public:
    USING_PTR (Sequence);

    std::vector<Stimulus::P> stimuli;
    std::vector<Response::P> responses;
};

#endif