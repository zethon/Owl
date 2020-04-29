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
    { "Seperator", O_COLOR_YELLOW, O_COLOR_BLACK },
    { "DebugInfo", O_COLOR_BLACK, O_COLOR_GREY },
};

///* If an xterm is resized the contents on your text windows might be messed up.
//To handle this gracefully you should redraw all the stuff based on the new
//height and width of the screen. When resizing happens, your program is sent
//a SIGWINCH signal. You should catch this signal and do redrawing accordingly.
//*/
//void resizeHandler(int sig)
//{
//    int h, w;
//
//    // this simply doesn't update h&w under OSX when using terminal
//    getmaxyx(stdscr, h, w);
//    fprintf(stderr, "Resizing: (h= %d, w= %d )\n", h, w);
//    fprintf(stderr, "Resizing: (LINES= %d, COLS= %d )\n", LINES, COLS);
//    refresh();
//}

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

    if (it == std::end(DEFAULT_THEME)) return 0;
    return static_cast<int>(std::distance(std::begin(DEFAULT_THEME), it));
}

int colornum(int fg, int bg)
{
    int B, bbb, ffff;

    B = 1 << 7;
    bbb = (7 & bg) << 4;
    ffff = 7 & fg;

    return (B | bbb | ffff);
}

[[maybe_unused]]
short curs_color(int fg)
{
    switch (7 & fg) {           /* RGB */
    case 0:                     /* 000 */
        return (COLOR_BLACK);
    case 1:                     /* 001 */
        return (COLOR_BLUE);
    case 2:                     /* 010 */
        return (COLOR_GREEN);
    case 3:                     /* 011 */
        return (COLOR_CYAN);
    case 4:                     /* 100 */
        return (COLOR_RED);
    case 5:                     /* 101 */
        return (COLOR_MAGENTA);
    case 6:                     /* 110 */
        return (COLOR_YELLOW);
    case 7:                     /* 111 */
        return (COLOR_WHITE);
    }

    return 0;
}

[[maybe_unused]] void init_colorpairs(void)
{
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(3, 0, 1000, 1000);
    init_color(6, 1000, 1000, 0);

    int fg, bg;
    int colorpair;

    for (bg = 0; bg <= 7; bg++) {
        for (fg = 0; fg <= 7; fg++) {
            colorpair = colornum(fg, bg);
            init_pair(colorpair, curs_color(fg), curs_color(bg));
        }
    }
}

int is_bold(int fg)
{
    /* return the intensity bit */

    int i;

    i = 1 << 3;
    return (i & fg);
}

[[maybe_unused]] void drawHorizontalLine(int x, int y, int length)
{
    move(y, x);
    for (int idx = 0; idx < length; idx++)
    {
        addch(ACS_HLINE);
        move(y, x + idx);
    }
}

[[maybe_unused]] void setcolor(int fg, int bg, bool bold = false)
{
    /* set the color pair (colornum) and bold/bright (A_BOLD) */

    attron(COLOR_PAIR(colornum(fg, bg)));
    if (is_bold(fg) || bold)
    {
        attron(A_BOLD);
    }
}

[[maybe_unused]] void unsetcolor(int fg, int bg, bool bold = false)
{
    /* unset the color pair (colornum) and
       bold/bright (A_BOLD) */

    attroff(COLOR_PAIR(colornum(fg, bg)));
    if (is_bold(fg) || bold)
    {
        attroff(A_BOLD);
    }
}

[[maybe_unused]] void printText(int fg, int bg, const std::string_view& text)
{
    setcolor(fg, bg);
    addstr(text.data());
    unsetcolor(fg, bg);
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

}

CursesApp::CursesApp()
{
    _window = initscr();
    curs_set(0); // hide cursor
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    //signal(SIGWINCH, resizeHandler);

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
            //case KEY_END:
            //{
            //    clear();
            //    printColors();
            //    clear();
            //    break;
            //}

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
        fmt::format("{:<{}}", "[?]Help [q]Quit [/]Prompt", std::get<0>(xy));
    addstr(menu.c_str());
    attroff(COLOR_PAIR(color_pair_num("MenuBar")));
}

void CursesApp::printHome()
{
    auto [x,y] = getScreenSize();

    ColorScope cs{ _window, color_pair_num("WelcomeText") };
    const std::string message = fmt::format(" :: {} :: ", APP_TITLE);
    cs.printXY(x - static_cast<int>(message.size() + 1), 0, message);

    const std::string debugInfo =
        fmt::format("X: {}, Y: {}, COLORS: {}, COLOR_PAIRS: {}", x, y, COLORS, COLOR_PAIRS);
    cs.reset(color_pair_num("DebugInfo"));
    cs.printXY(0, 0, debugInfo);

    cs.reset(color_pair_num("Seperator"));
    cs.drawHorizontalLine(10, 20, 30);
    cs.reset();

    printBottomMenu();
    move(y - 1, x - 1);
    refresh();
}

} // namespace
