#ifndef CAMERACONTROL_HPP
#define CAMERACONTROL_HPP

#include "Camera.hpp"
#include "Event.hpp"
#include "InputState.hpp"
#include "Window.hpp"

class CameraControl {
private:
    Camera&          camera;
    MouseState       mouse;
    KeyboardState::U keyboard;

public:
    CameraControl (Camera& camera, Window::Events& windowEvents)
        : camera (camera)
        , keyboard (KeyboardState::Create ())
    {
        windowEvents.mouseMove += std::bind (&CameraControl::ProcessMouseInput, this, std::placeholders::_1, std::placeholders::_2);
        windowEvents.keyPressed += std::bind (&KeyboardState::SetPressed, keyboard.get (), std::placeholders::_1);
        windowEvents.keyReleased += std::bind (&KeyboardState::SetReleased, keyboard.get (), std::placeholders::_1);
        windowEvents.leftMouseButtonPressed += [&] (auto...) { mouse.leftButton = true; };
        windowEvents.leftMouseButtonReleased += [&] (auto...) { mouse.leftButton = false; };

        windowEvents.resized += [&] (uint32_t width, uint32_t height) {
            const float aspect = static_cast<float> (width) / height;
            camera.SetAspectRatio (aspect);
        };
    }

    void UpdatePosition (float dt)
    {
        if (keyboard->IsPressed ('W')) {
            camera.Move (Camera::MovementDirection::Forward, dt);
        }
        if (keyboard->IsPressed ('A')) {
            camera.Move (Camera::MovementDirection::Left, dt);
        }
        if (keyboard->IsPressed ('S')) {
            camera.Move (Camera::MovementDirection::Backward, dt);
        }
        if (keyboard->IsPressed ('D')) {
            camera.Move (Camera::MovementDirection::Right, dt);
        }
        if (keyboard->IsPressed ('E')) {
            camera.Move (Camera::MovementDirection::Down, dt);
        }
        if (keyboard->IsPressed ('Q')) {
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