#include <signal.h>

#include "CursesApp.h"

namespace owl
{

/* If an xterm is resized the contents on your text windows might be messed up.
To handle this gracefully you should redraw all the stuff based on the new
height and width of the screen. When resizing happens, your program is sent
a SIGWINCH signal. You should catch this signal and do redrawing accordingly.
*/
void resizeHandler(int sig)
{
    int h, w;

    // this simply doesn't update h&w under OSX when using terminal
    getmaxyx(stdscr, h, w);
    fprintf(stderr, "Resizing: (h= %d, w= %d )\n", h, w);
    fprintf(stderr, "Resizing: (LINES= %d, COLS= %d )\n", LINES, COLS);
    refresh();
}

CursesApp::CursesApp()
{
    _window = initscr();

    signal(SIGWINCH, resizeHandler);
}

void CursesApp::run()
{
    printw("Oh hi!");
    getch();
    endwin();
}

} // namespace