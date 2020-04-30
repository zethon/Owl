#include "ColorScope.h"

#include <curses.h>

namespace owl
{

ColorScope::ColorScope(WINDOW* window, int colornum, bool bold)
    : _window{ window }, _colornum{ colornum }, _bold{ bold }
{
    turnOnAttributes();
}

ColorScope::~ColorScope()
{
    turnOffAttributes();
}

void ColorScope::reset(int colornum, bool bold)
{
    turnOffAttributes();

    _colornum = colornum;
    _bold = bold;

    turnOnAttributes();
    _reset = false;
}

void ColorScope::reset()
{
    turnOffAttributes();
}

void ColorScope::print(const std::string_view& text)
{
    waddstr(_window, text.data());
}

void ColorScope::printXY(int x, int y, std::string_view text)
{
    int origx = 0;
    int origy = 0;
    getyx(_window, origy, origx);

    wmove(_window, y, x);
    waddstr(_window, text.data());
    wmove(_window, origy, origx);
}

void ColorScope::drawHorizontalLine(int x, int y, int length)
{
    wmove(_window, y, x);
    whline(_window, ACS_HLINE, length);
}

void ColorScope::turnOnAttributes()
{
    wattron(_window, COLOR_PAIR(_colornum));
    if (_bold) wattron(_window, A_BOLD);
    _reset = false;
}

void ColorScope::turnOffAttributes()
{
    if (_reset) return;
    wattroff(_window, COLOR_PAIR(_colornum));
    if (_bold) wattroff(_window, A_BOLD);
    _reset = true;
}

int color_pair(std::string_view name)
{
    const auto it = std::find_if(
                std::begin(DEFAULT_THEME), std::end(DEFAULT_THEME),
                [name](const ColorPairInfo& info)
    {
        return name == info.name;
    });

    if (it == std::end(DEFAULT_THEME))
    {
        // TODO: warning?
        return 0;
    }

    return static_cast<int>(std::distance(std::begin(DEFAULT_THEME), it));
}

} // namespade
