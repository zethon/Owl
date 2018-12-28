#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QDebug>

#include "PaginationWidget.h"

namespace owl
{

static const char* strPaginationWidgetStyle = R"(
QWidget
{

}
)";

PaginationWidget::PaginationWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(strPaginationWidgetStyle);
    setMaximumHeight(128);
    setMinimumHeight(128);

    _prevAction = new QAction(this);
    _prevAction->setText(tr("Prev"));

    _nextAction = new QAction(this);
    _nextAction->setText(tr("Next"));

    _toolBar = new QToolBar(this);
    _toolBar->addAction(_prevAction);
    _toolBar->addSeparator();
    _toolBar->addAction(_nextAction);

    QObject::connect(_toolBar, &QToolBar::actionTriggered,
        [this](QAction* action)
        {
            std::uint32_t page = static_cast<std::uint32_t>(action->data().toInt());
            Q_EMIT doGotoPage(page);
        });


    QHBoxLayout* buttonLayout = new QHBoxLayout(parent);
    buttonLayout->addWidget(_toolBar);

    setLayout(buttonLayout);
}

void PaginationWidget::setPages(std::uint32_t current, std::uint32_t total)
{
    _currentPage = current;
    _totalPages = total;

    _prevAction->setVisible(_currentPage > 1);
    _prevAction->setData(_currentPage - 1);

    _nextAction->setVisible(_currentPage < _totalPages);
    _nextAction->setData(_currentPage + 1);
}

} // namespace
