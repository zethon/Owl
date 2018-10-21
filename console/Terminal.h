#pragma once
#include <QObject>
#include <mutex>

#ifdef _WINDOWS
#else
#include "terminalosx.h"
#endif

namespace owl
{

class Terminal final : public QObject
{
    Q_OBJECT

public:
    Terminal();
    virtual ~Terminal();

    void setEcho(bool b) { _bEcho = b; }
    bool getEcho() const { return _bEcho; }

    void setPrompt(bool b) { _bPrompt = b; }
    bool isPrompt() const { return _bPrompt; }

    void run();

Q_SIGNALS:
    void onChar(QChar c);
    void onBackspace();
    bool onEnter();

private:
    bool _bEcho = true;
    bool _bPrompt = false;

    std::pair<bool, char> getChar();
};

} // namespace
