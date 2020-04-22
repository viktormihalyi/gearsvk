#ifndef CAMERACONTROL_HPP
#define CAMERACONTROL_HPP

#include "Camera.hpp"
#include "Event.hpp"
#include "InputState.hpp"

class CameraControl {
public:
    struct Settings {
        Event<uint32_t>&           keyPressed;
        Event<uint32_t>&           keyReleased;
        Event<uint32_t, uint32_t>& leftMouseButtonPressed;
        Event<uint32_t, uint32_t>& leftMouseButtonReleased;
        Event<uint32_t, uint32_t>& mouseMove;
    };

private:
    Camera&       camera;
    MouseState    mouse;
    KeyboardState keyboard;
    Settings      settings;

public:
    CameraControl (Camera& camera, const Settings& settings)
        : camera (camera)
        , settings (settings)
    {
        settings.mouseMove += std::bind (&CameraControl::ProcessMouseInput, this, std::placeholders::_1, std::placeholders::_2);
        settings.keyPressed += std::bind (&KeyboardState::SetPressed, &keyboard, std::placeholders::_1);
        settings.keyReleased += std::bind (&KeyboardState::SetReleased, &keyboard, std::placeholders::_1);
        settings.leftMouseButtonPressed += [&] (auto...) { mouse.leftButton = true; };
        settings.leftMouseButtonReleased += [&] (auto...) { mouse.leftButton = false; };
    }

    void UpdatePosition (float dt)
    {
        if (keyboard.IsPressed ('W')) {
            camera.Move (Camera::MovementDirection::Forward, dt);
        }
        if (keyboard.IsPressed ('A')) {
            camera.Move (Camera::MovementDirection::Left, dt);
        }
        if (keyboard.IsPressed ('S')) {
            camera.Move (Camera::MovementDirection::Backward, dt);
        }
        if (keyboard.IsPressed ('D')) {
            camera.Move (Camera::MovementDirection::Right, dt);
        }
        if (keyboard.IsPressed ('E')) {
            camera.Move (Camera::MovementDirection::Down, dt);
        }
        if (keyboard.IsPressed ('Q')) {
            camera.Move (Camera::MovementDirection::Up, dt);
        }
    }

private:
    void ProcessMouseInput (uint32_t x, uint32_t y)
    {
        static glm::vec2 lastPos (x, y);
        glm::vec2        currentPos (x, y);
        if (mouse.leftButton) {
            camera.ProcessMouseInput (lastPos - currentPos);
        }
        lastPos = currentPos;
    }
};

#endif