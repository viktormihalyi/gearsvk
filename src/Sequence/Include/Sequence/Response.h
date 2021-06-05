#pragma once

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
        uint32_t        key;
        bool        visible;
    };
    std::vector<Button> buttons;
    unsigned int        duration; //frames
    unsigned int        startingFrame;

    std::shared_ptr<Sequence> sequence; //< Part of this sequence.


    Response ();
    virtual ~Response () = default;

    void addButton (std::string label, float x, float y, float w, float h, uint32_t key, bool visible);

    void setSequence (std::shared_ptr<Sequence> sequence)
    {
        this->sequence = sequence;
    }

    std::shared_ptr<Sequence> getSequence () { return sequence; }
};
