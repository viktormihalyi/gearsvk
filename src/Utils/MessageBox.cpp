#include "MessageBox.hpp"


#include <iostream>
#include <string>
#include <vector>

#include "Assert.hpp"

namespace MessageBox {

#ifdef GEARSVK_SDL_MESSAGEBOX

// #include <SDL.h>
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

    int buttonIndex = -1;
    if (SDL_ShowMessageBox (&messageBoxData, &buttonIndex) < 0) {
        return Result::Error;
    }

    if (buttonIndex < 0 || buttonIndex > static_cast<int> (buttons.size ()) - 1) {
        return Result::Error;
    }

    return buttonOrder[buttonIndex];
}

#elif _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

Result Show (const std::string& title, const std::string& message)
{
    const int msgBoxResult = MessageBox (NULL, message.c_str (), title.c_str (), MB_YESNOCANCEL);
    switch (msgBoxResult) {
        case IDYES: return Result::Yes;
        case IDNO: return Result::No;
        case IDCANCEL: return Result::Third;
        default: GVK_ASSERT (false); return Result::Error;
    }
}

#else

// "messagebox" in terminal

#include "TerminalColors.hpp"

Result Show (const std::string& title, const std::string& message)
{
    std::cout << title << std::endl;
    std::cout << "\t" << message << std::endl;

    char choice = 0;

    while (choice != 'y' && choice != 'n' && choice != 'i') {
        std::cout << "[y/n/i] ";
        std::cin >> choice;
        choice = std::tolower (choice);
    }

    if (choice == 'y') {
        return Result::Yes;
    } else if (choice == 'n') {
        return Result::No;
    } else if (choice == 'i') {
        return Result::Third;
    }

    return Result::No;
}

#endif

} // namespace MessageBox
