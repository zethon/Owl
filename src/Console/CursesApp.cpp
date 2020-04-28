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

    //signal(SIGWINCH, resizeHandler);

    if (!has_colors())
    {
        return;
    }

    start_color();

//    init_pair(NORMAL_TEXT, COLOR_WHITE, COLOR_BLACK);
//    init_pair(BRIGHT_TEXT, COLOR_WHITE + 8, COLOR_BLACK);
//    init_pair(YELLOW_TEXT, COLOR_YELLOW, COLOR_BLACK);
//    init_pair(HIGHLIGHT_TEXT, COLOR_WHITE, COLOR_YELLOW);

    init_color(666, 0, 0, 0);

    init_pair(APP_TITLE_TEXT, COLOR_MAGENTA , COLOR_BLACK);
    init_pair(FG_HIGHLIGHT, COLOR_YELLOW , COLOR_BLACK);
    init_pair(MENU_COLOR, 666, COLOR_WHITE);

//    int fg, bg;
//    int colorpair;

//    for (bg = 0; bg <= 7; bg++) {
//        for (fg = 0; fg <= 7; fg++) {
//            colorpair = colornum(fg, bg);
//            init_pair(colorpair, curs_color(fg), curs_color(bg));
//        }
//    }


}

void CursesApp::run()
{
//    printw("Oh hi!");
//    getch();
    printHome();
    getch();
    endwin();
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

    attron(COLOR_PAIR(MENU_COLOR));
    //attron(A_BOLD);

    const auto menu =
        fmt::format("{:<{}}", "[?]Help [q]Quit [/]Prompt", std::get<0>(xy));

    addstr(menu.c_str());

    attroff(COLOR_PAIR(MENU_COLOR));
    //attroff(A_BOLD);
}

void CursesApp::printHome()
{
    clear();
    auto [x,y] = getScreenSize();

    //const std::string debugInfo =
    //    fmt::format("X: {}, Y: {}, COLORS: {}", x, y, COLORS);
    //move(0, 0);
    //addstr(debugInfo.c_str());

    ColorScope cs{ _window, APP_TITLE_TEXT, true };
    const std::string message = fmt::format(" :: {} :: ", APP_TITLE);
    cs.printXY(x - static_cast<int>(message.size() + 1), 0, message);
    //move(0, x - static_cast<int>(message.size()+3));

    cs.reset(FG_HIGHLIGHT, true);
    cs.drawHorizontalLine(10, 20, 30);

    //attron(A_BOLD);
    //attron(COLOR_PAIR(APP_TITLE_TEXT));
    //addstr(message.c_str());
    //attroff(COLOR_PAIR(APP_TITLE_TEXT));

    //attron(COLOR_PAIR(FG_HIGHLIGHT));
    //
    //drawHorizontalLine(10, 20, 20);
    //attroff(COLOR_PAIR(FG_HIGHLIGHT));
    //attroff(A_BOLD);

    printBottomMenu();
    move(y - 1, x - 1);
    
    refresh();
}


//void CursesApp::printHome()
//{
//    clear();

//    auto [x, y] = this->getScreenSize();
    
//    std::string message = fmt::format(" .:: {} ::. ", APP_TITLE);
////    boost::algorithm::to_lower(message);
//    wmove(_window, 0, x - static_cast<int>(message.size()+3));




////    ColorScope cs(_window, COLOR_WHITE, COLOR_YELLOW, true);
////    cs.printXY(0, x - static_cast<int>(message.size()+3), message);

////    cs.reset(COLOR_YELLOW, COLOR_BLACK, false);
////    cs.drawHorizontalLine(10, 5, 20);
////    cs.drawHorizontalLine(10, 9, 20);


//////    drawHorizontalLine(_window, 10, 5, 20);
//////    drawHorizontalLine(_window, 10, 9, 20);
//    wmove(_window, y - 1, x - 1);
//}

} // namespace
