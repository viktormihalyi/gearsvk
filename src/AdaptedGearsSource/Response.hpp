#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Utils.hpp"

class Response : public Noncopyable {
public:
    USING_PTR (Response);

    struct Button {
        std::string label;
        float       xcoord, ycoord, width, height;
        uint        key;
        bool        visible;
    };

    std::string         question;
    uint32_t            duration;
    uint32_t            startingFrame;
    bool                loop;
    std::vector<Button> buttons;
};

#endif