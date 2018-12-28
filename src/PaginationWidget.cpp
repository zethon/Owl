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
    setMaximumHeight(64);
    setMinimumHeight(64);

    _prevAction = new QAction(this);
    _prevAction->setText(tr("Prev"));

    _nextAction = new QAction(this);
    _nextAction->setText(tr("Next"));

    for (auto x = 0; x < 9; x++)
    {
        _actionList.push_back(new QAction(this));
        _actionList.back()->setText(QString::number(x));
    }

    _toolBar = new QToolBar(this);
    _toolBar->addAction(_prevAction);
    _toolBar->addActions(_actionList);
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
