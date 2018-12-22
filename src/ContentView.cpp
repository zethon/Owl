#include <QLabel>
#include <QGridLayout>

#include  <Utils/OwlLogger.h>

#include "ContentView.h"

namespace owl
{

//********************************
//* LogoView
//********************************

LogoView::LogoView(QWidget *parent)
    : QWidget(parent)
{
    _bgImg = new QLabel(this);
    _bgImg->setPixmap(QPixmap(":/images/owl-bg1.png"));

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(_bgImg, 0, 0);

    this->setLayout(layout);
}
//********************************
//* ContentView
//********************************

ContentView::ContentView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("ContentView") }
{
    _logoView = new LogoView(this);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(_logoView);

    this->setLayout(layout);
}

} // namespace
