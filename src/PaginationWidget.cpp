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

static const char* strPaginationWidgetStyle = R"(
QToolBar
{
    background: yellow;
}
QToolButton#currentPage
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

    QObject::connect(_toolBar, &QToolBar::actionTriggered,
        [](QAction* action)
        {
            std::uint32_t page = static_cast<std::uint32_t>(action->data().toInt());
            Q_UNUSED(page);
            Q_ASSERT(0);
//            Q_EMIT doGotoPage(page);
        });


//    _toolBar->addWidget(_prevButton);
//    for (std::uint32_t x = 0; x < totalPageButtons; x++)
//    {
//        auto button = new QToolButton(this);
//        button->setObjectName(QString::number(x));
//        button->setDefaultAction(new QAction(QString::number(x), this));
//        QObject::connect(button, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

//        _buttonList.push_back(button);
//        _toolBar->addWidget(button);
//    }
//    _toolBar->addWidget(_nextButton);

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


void PaginationWidget::createPreviousButtons()
{
    std::uint32_t currentLabel = static_cast<std::uint32_t>(std::max(1, static_cast<std::int32_t>(_currentPage - anchorIdx)));
    for (std::uint32_t x = currentLabel; x < _currentPage; x++)
    {
        QToolButton* newButton = nullptr;

        if ((_currentPage > anchorIdx + 1)
            && ((x == currentLabel) || (x == currentLabel + 1)))
        {
            // the `1` button
            if (x == currentLabel)
            {
                newButton = new QToolButton(this);
                newButton->setText("1");

                QAction* action = new QAction(newButton);
                action->setData(1);
                newButton->addAction(action);
            }
            else if (x == currentLabel + 1)
            {
                newButton = new QToolButton(this);
                newButton->setText("?");
                newButton->addAction(new GotoPageWidgetAction(newButton));
                newButton->setPopupMode(QToolButton::InstantPopup);
            }
        }
        else
        {
            newButton = new QToolButton(this);
            newButton->setText(QString::number(x));

            QAction* action = new QAction(newButton);
            action->setData(x);
            newButton->addAction(action);
        }

        Q_ASSERT(newButton);
        _toolBar->addWidget(newButton);
    }
}


void PaginationWidget::createNextButtons()
{
    std::uint32_t currentLabel = _currentPage + 1;
    if (currentLabel < _totalPages)
    {
        for (std::uint32_t x = anchorIdx + 1; x < totalPageButtons; x++, currentLabel++)
        {
            if (x > _totalPages) break;

            QToolButton* newButton = nullptr;

            if ((_totalPages - _currentPage > anchorIdx)
                && ((x == totalPageButtons - 1) || (x == totalPageButtons - 2)))
            {
                // the last button on the far right
                if (x == totalPageButtons - 1)
                {
                    newButton = new QToolButton(_toolBar);
                    newButton->setText(QString::number(_totalPages));

                    QAction* action = new QAction(newButton);
                    action->setData(_totalPages);
                    newButton->addAction(action);
                }
                else if (x == totalPageButtons - 2)
                {
                    newButton = new QToolButton(this);
                    newButton->setText("?");
                    newButton->addAction(new GotoPageWidgetAction(newButton));
                    newButton->setPopupMode(QToolButton::InstantPopup);
                }
            }
            else
            {
                newButton = new QToolButton(_toolBar);
                newButton->setText(QString::number(currentLabel));

                QAction* action = new QAction(newButton);
                action->setData(currentLabel);
                newButton->addAction(action);
            }

            Q_ASSERT(newButton);
            _toolBar->addWidget(newButton);
        }
    }
}

void PaginationWidget::setPages(std::uint32_t current, std::uint32_t total)
{
    _currentPage = current;
    _totalPages = total;

//    setPrevButtons();
//    setNextButtons();

    _toolBar->clear();

    _prevButton->setVisible(_currentPage > 1);
    _prevButton->defaultAction()->setData(_currentPage - 1);
    _toolBar->addWidget(_prevButton);

//    // handle the buttons to the left of the anchor (if there are any)
//    std::uint32_t currentLabel = _currentPage - anchorIdx;
//    if (currentLabel > 0)
//    {
//        for (std::int32_t x = 0; x < static_cast<std::int32_t>(anchorIdx); x++, currentLabel++)
//        {
//            if (currentLabel < 1) break;

//            QToolButton* button = new QToolButton(_toolBar);
//            button->setText(QString::number(currentLabel));
//            button->setObjectName(QString::number(currentLabel));

//            QAction* action = new QAction("FOO", button);
//            action->setData(currentLabel);
//            button->addAction(action);

//            _toolBar->addWidget(button);
//        }
//    }

    createPreviousButtons();

    // add the anchor
    QToolButton* anchorButton = new QToolButton(_toolBar);
    anchorButton->setText(QString::number(_currentPage));
    anchorButton->setObjectName("currentPage");
    _toolBar->addWidget(anchorButton);

    // add the buttons ot the right of the anchor (if there are any)
    createNextButtons();

    _nextButton->setVisible(_currentPage < _totalPages);
    _nextButton->defaultAction()->setData(_currentPage + 1);
    _toolBar->addWidget(_nextButton);
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
