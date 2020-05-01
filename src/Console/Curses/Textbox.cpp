#include "Textbox.h"

namespace azc
{

Textbox::Textbox(WINDOW* window)
    : _window { window }
{
    // nothing to do
    keypad(_window, true);
}

Textbox::~Textbox()
{
    keypad(_window, false);
}

} // namespace curses
