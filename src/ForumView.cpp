#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>

#include  <Utils/OwlLogger.h>

#include "Data/Board.h"
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

    _tempLabel = new QLabel("THIS IS A LABEL", this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(_tempLabel);
    layout->addWidget(_treeView);

    setLayout(layout);
}

void ForumView::loadBoard(const owl::BoardPtr board)
{
    std::stringstream ss;
    ss << board->getName().toStdString()
       << ":" << board->getUsername().toStdString();

    _tempLabel->setText(QString::fromStdString(ss.str()));
}

} // namespace
