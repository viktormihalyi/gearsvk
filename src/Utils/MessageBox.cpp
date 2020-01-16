#include "MessageBox.hpp"

#include <SDL.h>
#include <vector>


namespace MessageBox {

Result Show (const std::string& title, const std::string& message)
{
    static const std::vector<Result> buttonOrder = {
        Result::No,
        Result::Yes,
        Result::Third,
    };

    static const std::vector<SDL_MessageBoxButtonData> buttons = {
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No"},
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes"},
        {0, 2, "Ignore"},
    };

    const SDL_MessageBoxData messageBoxData = {
        SDL_MESSAGEBOX_INFORMATION,
        nullptr, // parent window

        title.c_str (),
        message.c_str (),

        static_cast<int> (buttons.size ()),
        buttons.data (),

        nullptr, // colorscheme
    };

    int buttonIndex;
    if (SDL_ShowMessageBox (&messageBoxData, &buttonIndex) < 0) {
        return Result::Error;
    }

    if (buttonIndex < 0 || buttonIndex > buttons.size () - 1) {
        return Result::Error;
    }

    return buttonOrder[buttonIndex];
}

} // namespace MessageBox
