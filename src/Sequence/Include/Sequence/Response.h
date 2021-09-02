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

#ifdef GEARSVK_CEREAL
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#endif

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
        
#ifdef GEARSVK_CEREAL
        template<class Archive>
        void serialize (Archive& ar)
        {
            ar (CEREAL_NVP (label));
            ar (CEREAL_NVP (xcoord));
            ar (CEREAL_NVP (ycoord));
            ar (CEREAL_NVP (width));
            ar (CEREAL_NVP (height));
            ar (CEREAL_NVP (key));
            ar (CEREAL_NVP (visible));
        }
#endif
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

#ifdef GEARSVK_CEREAL
    template<class Archive>
    void serialize (Archive& ar)
    {
        ar (CEREAL_NVP (question));
        ar (CEREAL_NVP (loop));
        ar (CEREAL_NVP (buttons));
        ar (CEREAL_NVP (duration));
        ar (CEREAL_NVP (startingFrame));
        ar (CEREAL_NVP (sequence));
    }
#endif
};
