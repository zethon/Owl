#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QDebug>

#include "PaginationWidget.h"

#ifdef Q_OS_WIN
    #define LEFTARROW   "<-"
    #define RIGHTARROW  "->"
    #define ELLIPSIS    "..."
#else
    #define LEFTARROW   u8"\u2B05"
    #define RIGHTARROW  u8"\u27A1"
    #define ELLIPSIS    u8"\u2026"
#endif


namespace owl
{

// The total number of page buttons to be displayed, not counting the Previous
// and Next buttons
static const std::uint32_t totalPageButtons = 9;

// The postion of the current page when all `totalPageButtons` are displayed
static const std::uint32_t anchorIdx = 4;


static const char* strPaginationWidgetStyle = R"(
QToolBar
{
    background: transparent;
    spacing: 5px;
}
QToolButton
{
    font-size: 14px;
    color: rgb(143,143,143);
    border: 1px solid rgb(143,143,143);
    border-radius: 5px;
}
QToolButton:hover
{
    color: rgb(30,144,255);
    border: 1px solid rgb(143,143,143);
}
QToolButton::menu-indicator
{
    width:0px;
}
QToolButton#currentPage
{
    color: rgb(143,143,143);
    background-color: rgb(231,245,229);
}
QToolButton#currentPage:hover
{
}
)";

PaginationWidget::PaginationWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(strPaginationWidgetStyle);
    setMaximumHeight(32);
    setMinimumHeight(32);

    _buttonLayout = new QHBoxLayout();
    _buttonLayout->setMargin(0);
    _buttonLayout->setSpacing(0);
    setLayout(_buttonLayout);
}

void PaginationWidget::onButtonClicked(QAction* action)
{
    std::uint32_t page = static_cast<std::uint32_t>(action->data().toInt());
    Q_EMIT doGotoPage(page);
}

void PaginationWidget::setPages(std::uint32_t current, std::uint32_t total)
{
    Q_ASSERT(current > 0 && current <= total);

    _currentPage = current;
    _totalPages = total;

    if (_toolBar)
    {
        _buttonLayout->removeWidget(_toolBar);
        delete _toolBar;
    }

    _toolBar = new SimpleToolBar(this);
    _toolBar->setStyleSheet(strPaginationWidgetStyle);
    _buttonLayout->addWidget(_toolBar);

    createPreviousButtons();

    // add the anchor
    QToolButton* anchorButton = new QToolButton(_toolBar);
    anchorButton->setText(QString::number(_currentPage));
    anchorButton->setObjectName("currentPage");
    _toolBar->addWidget(anchorButton);

    // add the buttons ot the right of the anchor (if there are any)
    createNextButtons();
}

void PaginationWidget::createPreviousButtons()
{
    if (_currentPage > 1)
    {
        QToolButton* prevButton = new QToolButton(this);
        QObject::connect(prevButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

        prevButton->setDefaultAction(new QAction(LEFTARROW, prevButton));
        prevButton->defaultAction()->setData(_currentPage - 1);
        _toolBar->addWidget(prevButton);
    }

    std::uint32_t currentLabel = static_cast<std::uint32_t>(std::max(1, static_cast<std::int32_t>(_currentPage - anchorIdx)));
    for (std::uint32_t x = currentLabel; x < _currentPage; x++)
    {
        QToolButton* newButton = new QToolButton(_toolBar);

        if ((_currentPage > anchorIdx + 1)
            && ((x == currentLabel) || (x == currentLabel + 1)))
        {
            // the `1` button
            if (x == currentLabel)
            {
                QAction* action = new QAction(newButton);
                action->setText("1");
                action->setData(1);

                newButton->setDefaultAction(action);
                QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
            }
            else if (x == currentLabel + 1)
            {
                newButton->setText(ELLIPSIS);
                newButton->setPopupMode(QToolButton::InstantPopup);
                newButton->setObjectName(QStringLiteral("popup"));

                auto widgetAction = new GotoPageWidgetAction(_totalPages, newButton);
                QObject::connect(widgetAction, &GotoPageWidgetAction::gotoPage,
                    [this](std::uint32_t pageNumber) { Q_EMIT doGotoPage(pageNumber); });

                newButton->addAction(widgetAction);
            }
        }
        else
        {
            QAction* action = new QAction(newButton);
            action->setText(QString::number(x));
            action->setData(x);

            newButton->setDefaultAction(action);
            QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
        }

        Q_ASSERT(newButton);

        QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
        _toolBar->addWidget(newButton);
    }
}

void PaginationWidget::createNextButtons()
{
    std::uint32_t currentLabel = _currentPage + 1;

    for (std::uint32_t x = anchorIdx + 1; x < totalPageButtons; x++, currentLabel++)
    {
        if (currentLabel > _totalPages) break;

        QToolButton* newButton = new QToolButton(_toolBar);

        if ((_totalPages - _currentPage > anchorIdx)
            && ((x == totalPageButtons - 1) || (x == totalPageButtons - 2)))
        {
            // the last button on the far right
            if (x == totalPageButtons - 1)
            {
                QAction* action = new QAction(newButton);
                action->setText(QString::number(_totalPages));
                action->setData(_totalPages);

                newButton->setDefaultAction(action);
                QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
            }
            else if (x == totalPageButtons - 2)
            {
                newButton->setText(ELLIPSIS);
                newButton->setPopupMode(QToolButton::InstantPopup);
                newButton->setObjectName(QStringLiteral("popup"));

                auto widgetAction = new GotoPageWidgetAction(_totalPages, newButton);
                QObject::connect(widgetAction, &GotoPageWidgetAction::gotoPage,
                    [this](std::uint32_t pageNumber) { Q_EMIT doGotoPage(pageNumber); });

                newButton->addAction(widgetAction);
            }
        }
        else
        {
            QAction* action = new QAction(newButton);
            action->setText(QString::number(currentLabel));
            action->setData(currentLabel);

            newButton->setDefaultAction(action);
            QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
        }

        _toolBar->addWidget(newButton);
    }

    if (_currentPage < _totalPages)
    {
        QToolButton* nextButton = new QToolButton(this);
        QObject::connect(nextButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

        nextButton->setDefaultAction(new QAction(RIGHTARROW, nextButton));
        nextButton->defaultAction()->setData(_currentPage + 1);
        _toolBar->addWidget(nextButton);
    }
}

GotoPageWidgetAction::GotoPageWidgetAction(uint32_t totalPages, QWidget *parent)
    : QWidgetAction(parent),
      _totalPages(totalPages)
{}

QWidget *GotoPageWidgetAction::createWidget(QWidget *parent)
{
    GotoPageWidget* widget = new GotoPageWidget(_totalPages, parent);
    QObject::connect(widget, SIGNAL(gotoPage(std::uint32_t)), this, SIGNAL(gotoPage(std::uint32_t)));
    return widget;
}

GotoPageWidget::GotoPageWidget(uint32_t totalPages, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout*  layout = new QHBoxLayout(parent);

    QLabel* label = new QLabel(this);
    label->setText("Go to page");

    QLineEdit* edit = new QLineEdit(this);

    QIntValidator* validator = new QIntValidator(edit);
    validator->setBottom(1);
    validator->setTop(static_cast<int>(totalPages));
    edit->setValidator(validator);

    QPushButton* okBtn = new QPushButton(this);
    okBtn->setText("GO");
    QObject::connect(okBtn, &QPushButton::clicked,
        [edit,this]()
        {
            std::uint32_t pageNumber = static_cast<std::uint32_t>(edit->text().toInt());
            Q_EMIT gotoPage(pageNumber);
        });

    layout->addWidget(label);
    layout->addWidget(edit);
    layout->addWidget(okBtn);

    setLayout(layout);
}

SimpleToolBar::SimpleToolBar(QWidget *parent)
    : QWidget(parent)
{
    _layout = new QHBoxLayout();
    _layout->setMargin(0);
    _layout->setSpacing(0);

    setLayout(_layout);
}

void SimpleToolBar::addWidget(QWidget* widget)
{
    widget->setParent(this);
    _layout->addWidget(widget);
}

} // namespace
