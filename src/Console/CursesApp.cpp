#include <string>

#include <boost/algorithm/string.hpp>

#include <fmt/format.h>
//#include <signal.h>

#include "Core.h"
#include "CursesApp.h"

namespace owl
{

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

int colornum(int fg, int bg)
{
    int B, bbb, ffff;

    B = 1 << 7;
    bbb = (7 & bg) << 4;
    ffff = 7 & fg;

    return (B | bbb | ffff);
}

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

CursesApp::CursesApp()
{
    _window = initscr();

    //signal(SIGWINCH, resizeHandler);

    if (!has_colors())
    {
        return;
    }

    start_color();

    int fg, bg;
    int colorpair;

    for (bg = 0; bg <= 7; bg++) {
        for (fg = 0; fg <= 7; fg++) {
            colorpair = colornum(fg, bg);
            init_pair(colorpair, curs_color(fg), curs_color(bg));
        }
    }
}

void CursesApp::run()
{
    printw("Oh hi!");
    getch();
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

void CursesApp::printHome()
{
    auto [x, y] = this->getScreenSize();
    
    std::string message = fmt::format("{}", APP_TITLE);
    boost::algorithm::to_lower(message);
    wmove(_window, 0, x - static_cast<int>(message.size()+1));

    attron(COLOR_PAIR(colornum(COLOR_WHITE, COLOR_CYAN)));
    addstr(message.c_str());
    refresh();
    attroff(COLOR_PAIR(colornum(COLOR_WHITE, COLOR_CYAN)));
}

} // namespace