#ifndef INPUTSTATE_HPP
#define INPUTSTATE_HPP

#include "Assert.hpp"
#include "Ptr.hpp"

#include <array>

//#define ASSERT_ON_KEYS ASSERT
#define ASSERT_ON_KEYS

class KeyboardState {
private:
    static const uint32_t KEYCOUNT = 1024;

    std::array<bool, KEYCOUNT> pressedKeys;

public:
    USING_PTR (KeyboardState);

    KeyboardState ()
    {
        pressedKeys.fill (false);
    }

    bool IsPressed (uint32_t index) const
    {
        if (ASSERT_ON_KEYS (index <= 1024)) {
            return pressedKeys[index];
        }

        return false;
    }

    void SetPressed (uint32_t index)
    {
        if (ASSERT_ON_KEYS (index <= 1024)) {
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