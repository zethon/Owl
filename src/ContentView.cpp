#include  <Utils/OwlLogger.h>

#include "ContentView.h"

namespace owl
{

//********************************
//* ContentView
//********************************

ContentView::ContentView(QWidget* parent /* = 0*/)
    : QWidget(parent),
      _logger { owl::initializeLogger("ContentView") }
{}

} // namespace