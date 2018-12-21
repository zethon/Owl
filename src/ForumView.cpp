#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
#include "Data/ForumTreeModel.h"

#include "ForumView.h"

namespace owl
{

//********************************
//* ForumView
//********************************

ForumView::ForumView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("ForumView") }
{

    _treeView = new QTreeView(this);
    _treeView->setAttribute(Qt::WA_MacShowFocusRect, false);


    _tempLabel = new QLabel(this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(_tempLabel);
    layout->addWidget(_treeView);

    setLayout(layout);
}

void ForumView::doBoardClicked(const owl::BoardWeakPtr boardWeakPtr)
{
    // The user clicked on a board, so we have to:
    // * if we are not connected, attempt to connect
    // * if the forum-tree needs to be refershed, refresh it
    // * if the forum-tree is not displayed, display it

    // We want to lock the pointer to the current board and the one
    // we were just passed in and test for equality
    {
        owl::BoardPtr currentBoard = _currentBoard.lock();
        owl::BoardPtr board = boardWeakPtr.lock();

        if (board && currentBoard)
        {
            if (*board != *currentBoard)
            {
                _currentBoard = boardWeakPtr;
            }
        }
        else if (!currentBoard && board)
        {
            _currentBoard = boardWeakPtr;
        }
    }

    owl::BoardPtr currentBoard = _currentBoard.lock();
    Q_ASSERT(currentBoard);

    ForumPtr root = currentBoard->getRootStructure(false);
    ForumTreeModel* model = new ForumTreeModel{ root };

    auto oldModel = _treeView->model();
    _treeView->setModel(model);
    if (oldModel) oldModel->deleteLater();

    std::stringstream ss;
    ss << currentBoard->getName().toStdString()
       << ":" << currentBoard->getUsername().toStdString();

    _tempLabel->setText(QString::fromStdString(ss.str()));
}

} // namespace
