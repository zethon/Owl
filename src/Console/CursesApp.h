#pragma once

#include <curses.h>

namespace owl
{

class CursesApp
{
    WINDOW* _window = nullptr;

public:
    CursesApp();

    void run();
};

} // namespace
