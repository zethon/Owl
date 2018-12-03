#include  <Utils/OwlLogger.h>

#include "BoardIconTree.h"

namespace owl
{

BoardIconTree::BoardIconTree(QWidget* parent /* = 0*/)
    : _logger { owl::initializeLogger("BoardIconTree") }
{
    //_logger->trace("Creating `BoardIconTree`");
    setFocusPolicy(Qt::TabFocus);
    setResizeMode(QQuickWidget::SizeRootObjectToView);

    setSource(QUrl("qrc:/qml/boardIconList.qml"));
}

} // namespace
