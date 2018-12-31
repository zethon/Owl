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

    _prevButton = new QToolButton(this);
    _prevButton->setDefaultAction(new QAction(tr("Prev"), this));
    QObject::connect(_prevButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

    _nextButton = new QToolButton(this);
    _nextButton->setDefaultAction(new QAction(tr("Next"), this));
    QObject::connect(_nextButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

    _toolBar = new QToolBar(this);
    _toolBar->setStyleSheet(strPaginationWidgetStyle);

    _toolBar->addWidget(_prevButton);
    for (std::uint32_t x = 0; x < totalPageButtons; x++)
    {
        auto button = new QToolButton(this);
        button->setObjectName(QString::number(x));
        button->setDefaultAction(new QAction(QString::number(x), this));
        QObject::connect(button, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

        _buttonList.push_back(button);
        _toolBar->addWidget(button);
    }
    _toolBar->addWidget(_nextButton);

    QHBoxLayout* buttonLayout = new QHBoxLayout(parent);
    buttonLayout->addWidget(_toolBar);

    setLayout(buttonLayout);
}

void PaginationWidget::onButtonClicked(QAction* action)
{
    std::uint32_t page = static_cast<std::uint32_t>(action->data().toInt());
    qDebug()  << "GOTO PAGE: " << page;
    Q_EMIT doGotoPage(page);
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
    _prevButton->setVisible(_currentPage > 1);
    _prevButton->defaultAction()->setData(_currentPage - 1);

    std::int32_t currentLabel = static_cast<std::int32_t>(_currentPage);
    for (std::int32_t x = anchorIdx; x >= 0; x--, currentLabel--)
    {
        _buttonList.at(x)->defaultAction()->setText(QString::number(currentLabel));
        _buttonList.at(x)->defaultAction()->setData(currentLabel);

        // NOTE: The position of the button in the `_toolBar` is offset by 1 because
        // of the `_prevButton`
        _toolBar->actions().at(x+1)->setVisible(currentLabel >= 1);
    }

    Q_ASSERT(_toolBar->actions().at(anchorIdx+1)->isVisible());

    // go through and hack the first two buttons so that the first
    // one always shows `1` and the second one becomes the goto
    // page dropdown
    if (_currentPage > anchorIdx+1)
    {
        // NOTE: the `+1` is because `lastPageIdx` refers to the index of the object in the
        // `_buttonList` but we need to check visibility in the `_toolbar` which is offset
        // by 1
        Q_ASSERT(_toolBar->actions().at(firstPageIdx+1)->isVisible());
        _buttonList.at(firstPageIdx)->defaultAction()->setText("1");
        _buttonList.at(firstPageIdx)->defaultAction()->setData(1);

        Q_ASSERT(_toolBar->actions().at(firstPageIdx+2)->isVisible());
        _buttonList.at(firstPageIdx+1)->defaultAction()->setText("?");
    }
}

void PaginationWidget::setNextButtons()
{
    Q_UNUSED(lastPageIdx);
    _nextButton->setVisible(_currentPage < _totalPages);
    _nextButton->defaultAction()->setData(_currentPage + 1);

    std::int32_t currentLabel = static_cast<std::int32_t>(_currentPage + 1);
    for (std::int32_t x = anchorIdx + 1; x < static_cast<std::int32_t>(totalPageButtons)
         ; x++, currentLabel++)
    {
        _buttonList.at(x)->defaultAction()->setText(QString::number(currentLabel));
        _buttonList.at(x)->defaultAction()->setData(currentLabel);

        // NOTE: The position of the button in the `_toolBar` is offset by 1 because
        // of the `_prevButton`
        _toolBar->actions().at(x+1)->setVisible(currentLabel <= static_cast<std::int32_t>(_totalPages));
    }

    Q_ASSERT(_toolBar->actions().at(anchorIdx+1)->isVisible());

    // go through and hack the last two buttons so that the last
    // one always shows the last page and the second to last
    // becomes the goto page dropdown
    if (_currentPage < (_totalPages - anchorIdx))
    {
        // NOTE: the `+1` is because `lastPageIdx` refers to the index of the object in the
        // `_buttonList` but we need to check visibility in the `_toolbar` which is offset
        // by 1
        Q_ASSERT(_toolBar->actions().at(lastPageIdx+1)->isVisible());
        _buttonList.at(lastPageIdx)->defaultAction()->setText(QString::number(_totalPages));
        _buttonList.at(lastPageIdx)->defaultAction()->setData(_totalPages);

        Q_ASSERT(_toolBar->actions().at(lastPageIdx)->isVisible());
        _buttonList.at(lastPageIdx-1)->setText("?");

//        QToolButton* button = new QToolButton(this);
//        button->setText("THIS");
//        button->addAction(new GotoPageWidgetAction(this));
//        button->setPopupMode(QToolButton::InstantPopup);
//        _toolBar->addWidget(button);
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
