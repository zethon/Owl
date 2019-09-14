#include <iostream>
#include <conio.h>

#include "Terminal.h"

namespace owl
{

Terminal::Terminal()
{
}

Terminal::~Terminal()
{
}

std::pair<bool, char> Terminal::getChar()
{
    return std::make_pair(true, 'c');
}

void Terminal::run()
{
    bool done = false;

    while (!done)
    {
        const int c = _getch();

        switch (c)
        {
            default:
            {
                onChar(c);
            }
            break;

            case '\b':
            {
                Q_SIGNAL onBackspace();
            }
            break;

            case '\r':
            {
                done = onEnter();
            }
            break;

            case 0:
            case 224:
            {
                switch (_getch())
                {
                    default:
                    break;

                    case 72:
                        Q_SIGNAL onUpArrow();
                    break;

                    case 80:
                        Q_SIGNAL onDownArrow();
                    break;

                    case 75:
                        Q_SIGNAL onLeftArrow();
                    break;

                    case 77:
                        Q_SIGNAL onRightArrow();
                    break;
                }
            }
            break;
        }
    }
}

void Terminal::print(const QString& text)
{
    std::cout << text.toStdString();
}

void Terminal::println(const QString& text)
{
    std::cout << text.toStdString() << '\n';
}

void Terminal::backspace()
{
    std::cout << "\b \b" << std::flush;
}

} // namespace
