#pragma once

#include <map>
#include <string_view>
#include <string>

#include <boost/range/adaptor/indexed.hpp>

#include <curses.h>

namespace owl
{

// not a great design but will do for now
int color_pair(std::string_view name);

constexpr int O_COLOR_BLACK = 0;
constexpr int O_COLOR_BLUE = 1;
constexpr int O_COLOR_GREEN = 2;
constexpr int O_COLOR_CYAN = 3;
constexpr int O_COLOR_RED = 4;
constexpr int O_COLOR_MAGENTA = 5;
constexpr int O_COLOR_YELLOW = 6;
constexpr int O_COLOR_WHITE = 7;
constexpr int O_COLOR_GREY = 8;

using ColorMap = std::map<int, std::tuple<int, int, int>>;
const ColorMap COLOR_CODES =
{
    { O_COLOR_BLACK,    { 0, 0, 0 } },
    { O_COLOR_BLUE,     { 0, 0, 1000 } },
    { O_COLOR_GREEN,    { 0, 1000, 0 } },
    { O_COLOR_CYAN,     { 0, 1000, 1000 } },
    { O_COLOR_RED,      { 1000, 0, 0 } },
    { O_COLOR_MAGENTA,  { 1000, 0, 1000 } },
    { O_COLOR_YELLOW,   { 1000, 1000, 0 } },
    { O_COLOR_WHITE,    { 0, 0, 0 } },
    { O_COLOR_GREY,     { 500, 500, 500 } },
};

struct ColorPairInfo
{
    std::string name;
    int         fg;
    int         bg;
};

const ColorPairInfo DEFAULT_THEME[]
{
    { "", 0, 0 }, // ncurses default (skipped)
    { "MenuBar", O_COLOR_BLACK, O_COLOR_CYAN },
    { "WelcomeText", O_COLOR_MAGENTA, O_COLOR_BLACK },
    { "BoxOutline", O_COLOR_YELLOW, O_COLOR_BLACK },
    { "DebugInfo", O_COLOR_BLACK, O_COLOR_GREY },
    { "Warning", O_COLOR_BLACK, O_COLOR_YELLOW },
    { "Error", O_COLOR_BLACK, O_COLOR_RED },
    { "Prompt", O_COLOR_YELLOW, O_COLOR_BLUE },
    { "MenuItem", O_COLOR_GREY, O_COLOR_BLACK },
    { "MenuItemSelected", O_COLOR_YELLOW, O_COLOR_BLACK },
};

class ColorScope
{
    WINDOW* _window = nullptr;
    int     _colornum = 0;
    bool    _bold = false;
    bool    _reset = false;

public:
    ColorScope(WINDOW* window, int colornum, bool bold);

    ColorScope(WINDOW* window, int colornum)
        : ColorScope(window, colornum, false)
    {}

    explicit ColorScope(int colornum)
        : ColorScope(stdscr, colornum)
    {}

    ColorScope(WINDOW* window, std::string_view name, bool bold)
        : ColorScope(window, color_pair(name), bold)
    {}

    ColorScope(WINDOW* window, std::string_view name)
        : ColorScope(window, name, false)
    {}

    ColorScope(std::string_view name)
        : ColorScope(stdscr, color_pair(name))
    {}

    ~ColorScope();

    void reset(int colornum, bool bold = false);
    void reset();

    void print(const std::string_view& text);
    void printXY(int x, int y, std::string_view text);
    void drawHorizontalLine(int x, int y, int length);

private:
    void turnOnAttributes();
    void turnOffAttributes();
};

} // namespace
