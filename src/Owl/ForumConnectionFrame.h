#pragma once

#include <QFrame>

#include <Data/Board.h>

#include "ConnectionFrame.h"

namespace owl
{

class ForumView;
class ContentView;

class ForumConnectionFrame : public ConnectionFrame
{
    Q_OBJECT

public:
    ForumConnectionFrame(owl::BoardPtr board, QWidget *parent = nullptr);
    owl::BoardWeakPtr board() const { return _board; }

Q_SIGNALS:

private:
    void setupUI();
    void setupSignals();
    void setupBoardSignals();

    void onLoginHandler(const owl::StringMap& info);

    owl::BoardWeakPtr   _board;
    owl::ContentView*   _forumContentView = nullptr;
    owl::ForumView*     _forumNavigationView = nullptr;
};

} // namspace owl
