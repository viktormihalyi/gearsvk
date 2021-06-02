#pragma once

#include "stdafx.h"
#include "SequenceAPI.hpp"
#include <memory>

#include <algorithm>

#include <iomanip>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class Sequence;

//! A structure that contains all stimulus parameters.
class SEQUENCE_API Response : public std::enable_shared_from_this<Response> {
public:
    std::string question;
    bool        loop;

    struct Button {
        std::string label;
        float       xcoord, ycoord, width, height;
        uint        key;
        bool        visible;
    };
    std::vector<Button> buttons;
    unsigned int        duration; //frames
    unsigned int        startingFrame;

    std::shared_ptr<Sequence> sequence; //< Part of this sequence.


    Response ();
    virtual ~Response () = default;

    GEARS_SHARED_CREATE (Response);

    void addButton (std::string label, float x, float y, float w, float h, uint key, bool visible);

    void setSequence (std::shared_ptr<Sequence> sequence)
    {
        this->sequence = sequence;
    }

    std::shared_ptr<Sequence> getSequence () { return sequence; }
};
