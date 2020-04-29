#include <iostream>
#include <string>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <fmt/format.h>
//#include <signal.h>

#include "Core.h"
#include "CursesApp.h"

namespace owl
{

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
};

namespace
{

// not a great design but will do for now
int color_pair_num(std::string_view name)
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

[[maybe_unused]] int is_bold(int fg)
{
    /* return the intensity bit */

    int i;

    i = 1 << 3;
    return (i & fg);
}

class ColorScope
{
    WINDOW* _window = nullptr;
    int     _colornum = 0;
    bool    _bold = false;
    bool    _reset = false;

public:
    ColorScope(WINDOW* window, int colornum, bool bold)
        : _window{ window }, _colornum{ colornum }, _bold{ bold }
    {
        turnOnAttributes();
    }

    ColorScope(WINDOW* window, int colornum)
        : ColorScope(window, colornum, false)
    {}

    explicit ColorScope(int colornum)
        : ColorScope(stdscr, colornum)
    {}

    ColorScope(WINDOW* window, std::string_view name, bool bold)
        : ColorScope(window, color_pair_num(name), bold)
    {}

    ColorScope(WINDOW* window, std::string_view name)
        : ColorScope(window, name, false)
    {}

    ColorScope(std::string_view name)
        : ColorScope(stdscr, color_pair_num(name))
    {}

    ~ColorScope()
    {
        turnOffAttributes();
    }

    void reset(int colornum, bool bold = false)
    {
        turnOffAttributes();

        _colornum = colornum;
        _bold = bold;

        turnOnAttributes();
        _reset = false;
    }

    void reset()
    {
        turnOffAttributes();
    }

    void print(const std::string_view& text)
    {
        waddstr(_window, text.data());
    }

    void printXY(int x, int y, std::string_view text)
    {
        int origx = 0;
        int origy = 0;
        getyx(_window, origy, origx);

        wmove(_window, y, x);
        waddstr(_window, text.data());
        wmove(_window, origy, origx);
    }

    void drawHorizontalLine(int x, int y, int length)
    {
        wmove(_window, y, x);
        whline(_window, ACS_HLINE, length);
    }

private:
    void turnOnAttributes()
    {
        wattron(_window, COLOR_PAIR(_colornum));
        if (_bold) wattron(_window, A_BOLD);
        _reset = false;
    }

    void turnOffAttributes()
    {
        if (_reset) return;
        wattroff(_window, COLOR_PAIR(_colornum));
        if (_bold) wattroff(_window, A_BOLD);
        _reset = true;
    }
};

void init_color_scheme()
{
    start_color();

    for (const auto& [code, rgb] : COLOR_CODES)
    {
        const auto& [r, g, b] = rgb;
        init_color(code, r, g, b);
    }

    for (const auto& element : boost::adaptors::index(DEFAULT_THEME)) 
    {
        if (element.index() == 0) continue;
        const auto& [name, fg, bg] = element.value();

        init_pair(static_cast<int>(element.index()), fg, bg);
    }
}

void print_color_settings()
{
    clear();

    constexpr int startX = 2;
    int startY = 2;

    for (const auto& element : boost::adaptors::index(DEFAULT_THEME))
    {
        if (element.index() == 0) continue;

        move(startY + element.index(), startX);
        const ColorPairInfo& info = element.value();
        attron(COLOR_PAIR(element.index()));
        addstr(fmt::format("{:^30}", info.name, 10).c_str());
        attroff(COLOR_PAIR(element.index()));
    }

    refresh();
    getch();
}

}

CursesApp::CursesApp()
{
    _window = initscr();
    curs_set(0); // hide cursor
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    if (!has_colors())
    {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    init_color_scheme();
}

CursesApp::~CursesApp()
{
    endwin();
}

void CursesApp::run()
{
    bool done = false;

    while (!done)
    {
        printHome();

        auto ch = getch();
        switch (ch)
        {
            case KEY_END:
            {
                print_color_settings();
                clear();
                break;
            }

            case 'q':
            case 'Q':
            {
                done = true;
                break;
            }
        }
    }
}

std::tuple<int, int> CursesApp::getScreenSize() const
{
    int x = 0;
    int y = 0;
    getmaxyx(_window, y, x);
    return { x, y };
}

void CursesApp::printBottomMenu()
{
    auto xy = getScreenSize();
    auto y = std::get<1>(xy);

    move(y-1, 0);

    attron(COLOR_PAIR(color_pair_num("MenuBar")));
    const auto menu =
        fmt::format("{:<{}}", "[?]Help [q]Quit [/]Prompt [u]Login", std::get<0>(xy));
    addstr(menu.c_str());
    attroff(COLOR_PAIR(color_pair_num("MenuBar")));
}

void CursesApp::printHome()
{
    auto [x,y] = getScreenSize();

    ColorScope cs{ _window, color_pair_num("WelcomeText"), true };
    const std::string message = fmt::format(" :: {} :: ", APP_TITLE);
    cs.printXY(x - static_cast<int>(message.size() + 1), 0, message);

    const std::string debugInfo =
        fmt::format("X: {}, Y: {}, COLORS: {}, COLOR_PAIRS: {}", x, y, COLORS, COLOR_PAIRS);
    cs.reset(color_pair_num("DebugInfo"));
    cs.printXY(0, 0, debugInfo);
//    cs.reset();

//    cs.reset(color_pair_num("Seperator"));
    {
//        auto smallwin = newwin(2, 2, 5, 5);
        auto win = newwin(2, 2, 5, 5);
        ColorScope smallcs{ win, color_pair_num("BoxOutline") };
        box(win, 0, 0);
//        smallcs.printXY(2, 2, "Hi there!");
    }
//    move(10,20);
//    box(_window, 0, 0);


    cs.reset(color_pair_num("BoxOutline"));
    cs.drawHorizontalLine(10, 20, 30);
//    cs.reset();

    printBottomMenu();
    move(y - 1, x - 1);
    refresh();
}

} // namespace
