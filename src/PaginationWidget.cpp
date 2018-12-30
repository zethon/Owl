#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>

#include "PaginationWidget.h"

namespace owl
{


// NOTE: This is the index of the `QAction` that is always
// the currently selected. All other `QAction`'s are either
// visible or invisible depending on where in the pagination
// we are
static const std::uint32_t anchorIdx = 4;

static const std::uint32_t totalPageButtons = 9;

// When we're in the middle of pagination, the first
// and last page are always displayed
static const std::uint32_t firstPageIdx = 0;
static const std::uint32_t lastPageIdx = totalPageButtons - 1;

// NOTE: The 4 in `QToolButton#4` refers to `anchorIdx`
static const char* strPaginationWidgetStyle = R"(
QToolBar
{
    background: yellow;
}
QToolButton#4
{
    background: red;
    text-decoration: underline;
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
    _prevAction->setObjectName("previous");

    _nextAction = new QAction(this);
    _nextAction->setText(tr("Next"));
    _prevAction->setObjectName("next");

    _toolBar = new QToolBar(this);
    _toolBar->setStyleSheet(strPaginationWidgetStyle);

    _toolBar->addAction(_prevAction);
    for (std::uint32_t x = 0; x < totalPageButtons; x++)
    {
        _actionList.push_back(new QAction(this));
        _actionList.back()->setText(QString::number(x));
        _actionList.back()->setObjectName(QString::number(x));
        _toolBar->addAction(_actionList.back());
        _toolBar->widgetForAction(_actionList.back())->setObjectName(QString::number(x));
    }
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

    setPrevButtons();
    setNextButtons();
}

void PaginationWidget::setPrevButtons()
{
    _prevAction->setVisible(_currentPage > 1);
    _prevAction->setData(_currentPage - 1);

    std::int32_t currentLabel = static_cast<std::int32_t>(_currentPage);
    for (std::int32_t x = anchorIdx; x >= 0; x--, currentLabel--)
    {
        _actionList.at(x)->setText(QString::number(currentLabel));
        _actionList.at(x)->setData(currentLabel);
        _actionList.at(x)->setVisible(currentLabel >= 1);
    }

    Q_ASSERT(_actionList.at(anchorIdx)->isVisible());

    // go through and hack the first two buttons so that the first
    // one always shows `1` and the second one becomes the goto
    // page dropdown
    if (_currentPage > anchorIdx+1)
    {
        Q_ASSERT(_actionList.at(firstPageIdx)->isVisible());
        _actionList.at(firstPageIdx)->setText("1");
        _actionList.at(firstPageIdx)->setData(1);

        Q_ASSERT(_actionList.at(firstPageIdx+1)->isVisible());
        _actionList.at(firstPageIdx+1)->setText("?");
        _actionList.at(firstPageIdx+1)->setData(1);
    }
}

void PaginationWidget::setNextButtons()
{
    _nextAction->setVisible(_currentPage < _totalPages);
    _nextAction->setData(_currentPage + 1);

    std::int32_t currentLabel = static_cast<std::int32_t>(_currentPage + 1);
    for (std::int32_t x = anchorIdx + 1; x < static_cast<std::int32_t>(totalPageButtons)
         ; x++, currentLabel++)
    {
        _actionList.at(x)->setText(QString::number(currentLabel));
        _actionList.at(x)->setData(currentLabel);
        _actionList.at(x)->setVisible(currentLabel <= static_cast<std::int32_t>(_totalPages));
    }

    Q_ASSERT(_actionList.at(anchorIdx)->isVisible());

    // go through and hack the last two buttons so that the last
    // one always shows the last page and the second to last
    // becomes the goto page dropdown
    if (_currentPage < (_totalPages - anchorIdx))
    {
        Q_ASSERT(_actionList.at(lastPageIdx)->isVisible());
        _actionList.at(lastPageIdx)->setText(QString::number(_totalPages));
        _actionList.at(lastPageIdx)->setData(_totalPages);

        Q_ASSERT(_actionList.at(lastPageIdx-1)->isVisible());
        _actionList.at(lastPageIdx-1)->setText("?");
        _actionList.at(lastPageIdx-1)->setData(1);

        QToolButton* button = new QToolButton(this);
        button->setText("THIS");
        button->setPopupMode(QToolButton::InstantPopup);
        button->setDefaultAction(new GotoPageWidgetAction(this));

        _toolBar->addWidget(button);
    }
}

GotoPageWidgetAction::GotoPageWidgetAction(QWidget *parent)
    : QWidgetAction(parent)
{}

QWidget *GotoPageWidgetAction::createWidget(QWidget *parent)
{
    GotoPageWidget* widget = new GotoPageWidget(parent);
    return widget;
}

GotoPageWidget::GotoPageWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout*  layout = new QHBoxLayout(parent);

    QLabel* label = new QLabel(this);
    label->setText("Go to page");

    QLineEdit* edit = new QLineEdit(this);

    QPushButton* okBtn = new QPushButton(this);
    okBtn->setText("GO");

    layout->addWidget(label);
    layout->addWidget(edit);
    layout->addWidget(okBtn);

    setLayout(layout);
}


} // namespace
