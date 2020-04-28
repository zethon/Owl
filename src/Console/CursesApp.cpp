#include <iostream>
#include <string>

#include <boost/algorithm/string.hpp>

#include <fmt/format.h>
//#include <signal.h>

#include "Core.h"
#include "CursesApp.h"

namespace owl
{

constexpr auto APP_TITLE_TEXT = 1l;
constexpr auto FG_HIGHLIGHT = 2u;
constexpr auto MENU_COLOR = 10u;
//constexpr auto NORMAL_TEXT = 1u;
//constexpr auto BRIGHT_TEXT = 2u;
//constexpr auto YELLOW_TEXT = 3u;
//constexpr auto HIGHLIGHT_TEXT = 4u;


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

    void printXY(int x, int y, const std::string_view& text)
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

    start_color();
    init_colorpairs();

//    init_pair(NORMAL_TEXT, COLOR_WHITE, COLOR_BLACK);
//    init_pair(BRIGHT_TEXT, COLOR_WHITE + 8, COLOR_BLACK);
//    init_pair(YELLOW_TEXT, COLOR_YELLOW, COLOR_BLACK);
//    init_pair(HIGHLIGHT_TEXT, COLOR_WHITE, COLOR_YELLOW);

//    init_color(666, 0, 0, 0);
//    init_color(777, 256, 0, 0);

//    init_pair(APP_TITLE_TEXT, COLOR_MAGENTA , COLOR_BLACK);
//    init_pair(FG_HIGHLIGHT, COLOR_YELLOW , COLOR_BLACK);
//    init_pair(MENU_COLOR, COLOR_YELLOW, COLOR_BLACK);

//    clear();


//    int fg, bg;
//    int colorpair;

//    for (bg = 0; bg <= 7; bg++) {
//        for (fg = 0; fg <= 7; fg++) {
//            colorpair = colornum(fg, bg);
//            init_pair(colorpair, curs_color(fg), curs_color(bg));
//        }
//    }




}

CursesApp::~CursesApp()
{
    endwin();
}



void printColors()
{
    if ((LINES < 24) || (COLS < 80))
    {
        endwin();
        puts("Your terminal needs to be at least 80x24");
        exit(2);
    }

    mvaddstr(0, 35, "COLOR DEMO");
    mvaddstr(2, 0, "low intensity text colors (0-7)");
    mvaddstr(12, 0, "high intensity text colors (8-15)");

    for (int bg = 0; bg <= 7; bg++)
    {
        for (int fg = 0; fg <= 7; fg++)
        {
            setcolor(fg, bg);
            std::string message
                = fmt::format("F:{},B:{}", fg, bg);
            message = fmt::format("{:^{}}", message, 10);
            mvaddstr(fg + 3, bg * 10, message.c_str());
            unsetcolor(fg, bg);
        }

        for (int fg = 8; fg <= 15; fg++)
        {
            setcolor(fg, bg);
            std::string message
                = fmt::format("F:{},B:{}", fg, bg);
            message = fmt::format("{:^{}}", message, 10);
            mvaddstr(fg + 5, bg * 10, message.c_str());
            unsetcolor(fg, bg);
        }
    }

    mvaddstr(LINES - 1, 0, "press any key");

    refresh();
    getch();
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
                clear();
                printColors();
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

    waddch(_window, 'X' | A_UNDERLINE | COLOR_PAIR(MENU_COLOR));
}

void CursesApp::printHome()
{
    auto [x,y] = getScreenSize();

    const std::string debugInfo =
        fmt::format("X: {}, Y: {}, COLORS: {}, COLOR_PAIRS: {}", x, y, COLORS, COLOR_PAIRS);
    move(0, 0);
    addstr(debugInfo.c_str());

    ColorScope cs{ _window, APP_TITLE_TEXT, true };
    const std::string message = fmt::format(" :: {} :: ", APP_TITLE);
    cs.printXY(x - static_cast<int>(message.size() + 1), 0, message);
    //move(0, x - static_cast<int>(message.size()+3));

    cs.reset(FG_HIGHLIGHT, true);
    cs.drawHorizontalLine(10, 20, 30);
    cs.reset();

    printBottomMenu();
    refresh();
    move(y - 1, x - 1);
    
    refresh();
}

} // namespace
