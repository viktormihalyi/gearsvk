#ifndef INPUTSTATE_HPP
#define INPUTSTATE_HPP

#include "Assert.hpp"

#include <array>

class KeyboardState {
private:
    static const uint32_t KEYCOUNT = 1024;

    std::array<bool, KEYCOUNT> pressedKeys;

public:
    KeyboardState ()
    {
        pressedKeys.fill (false);
    }

    bool IsPressed (uint32_t index) const
    {
        if (ASSERT (index <= 1024)) {
            return pressedKeys[index];
        }

        return false;
    }

    void SetPressed (uint32_t index)
    {
        if (ASSERT (index <= 1024)) {
            pressedKeys[index] = true;
        }
    }

    void SetReleased (uint32_t index)
    {
        if (ASSERT (index <= 1024)) {
            pressedKeys[index] = false;
        }
    }
};


class MouseState {
public:
    bool leftButton;
    bool rightButton;

    MouseState ()
        : leftButton (false)
        , rightButton (false)
    {
    }
};

#endif