#include <QVBoxLayout>
#include <QLabel>

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

    QLabel* label = new QLabel("THIS IS A LABEL", this);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(label);

    setLayout(layout);
}

void ForumView::loadBoard(const owl::BoardPtr board)
{
    qDebug() << "loading: " << board->getName() << ":" << board->getUsername();
}

} // namespace
