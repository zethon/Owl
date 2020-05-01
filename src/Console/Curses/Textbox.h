#pragma once

#include <curses.h>

namespace azc
{

class Textbox
{
    WINDOW* _window;
    [[maybe_unused]] int     _x = 0;
    [[maybe_unused]] int     _y = 0;

public:
    explicit Textbox(WINDOW* window);
    ~Textbox();

};


} // namespace curses
