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

class MyToolButton : public QToolButton
{
public:
    using QToolButton::QToolButton;
    ~MyToolButton()
    {
        qDebug() << "MY TOOLBUTTON DESTROYETH!";
    }
};

class MyAction : public QAction
{
public:
    using QAction::QAction;
    ~MyAction()
    {
        qDebug() << "MY ACTION DESTROYETH!";
    }
};

// The total number of page buttons to be displayed, not counting the Previous
// and Next buttons
static const std::uint32_t totalPageButtons = 9;

// The postion of the current page when all `totalPageButtons` are displayed
static const std::uint32_t anchorIdx = 4;


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

    _buttonLayout = new QHBoxLayout(parent);
    setLayout(_buttonLayout);
}

void PaginationWidget::onButtonClicked(QAction* action)
{
    std::uint32_t page = static_cast<std::uint32_t>(action->data().toInt());
    qDebug()  << "GOTO PAGE: " << page;
    Q_EMIT doGotoPage(page);
}


void PaginationWidget::createPreviousButtons()
{
    if (_currentPage > 1)
    {
        QToolButton* prevButton = new MyToolButton(this);
        QObject::connect(prevButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

        prevButton->setDefaultAction(new MyAction(tr("Prev"), prevButton));
        prevButton->defaultAction()->setData(_currentPage - 1);
        _toolBar->addWidget(prevButton);
    }

    std::uint32_t currentLabel = static_cast<std::uint32_t>(std::max(1, static_cast<std::int32_t>(_currentPage - anchorIdx)));
    for (std::uint32_t x = currentLabel; x < _currentPage; x++)
    {
        QToolButton* newButton = new MyToolButton(_toolBar);

        if ((_currentPage > anchorIdx + 1)
            && ((x == currentLabel) || (x == currentLabel + 1)))
        {
            // the `1` button
            if (x == currentLabel)
            {
                QAction* action = new MyAction(newButton);
                action->setText("1");
                action->setData(1);

                newButton->setDefaultAction(action);
            }
            else if (x == currentLabel + 1)
            {
                newButton->setText("?");
                newButton->addAction(new GotoPageWidgetAction(newButton));
                newButton->setPopupMode(QToolButton::InstantPopup);
            }
        }
        else
        {
            QAction* action = new MyAction(newButton);
            action->setText(QString::number(x));
            action->setData(x);

            newButton->setDefaultAction(action);
        }

        Q_ASSERT(newButton);

        QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
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

            QToolButton* newButton = new MyToolButton(_toolBar);

            if ((_totalPages - _currentPage > anchorIdx)
                && ((x == totalPageButtons - 1) || (x == totalPageButtons - 2)))
            {
                // the last button on the far right
                if (x == totalPageButtons - 1)
                {
                    QAction* action = new MyAction(newButton);
                    action->setText(QString::number(_totalPages));
                    action->setData(_totalPages);

                    newButton->setDefaultAction(action);
                }
                else if (x == totalPageButtons - 2)
                {
                    newButton->setText("?");
                    newButton->addAction(new GotoPageWidgetAction(newButton));
                    newButton->setPopupMode(QToolButton::InstantPopup);
                }
            }
            else
            {
                QAction* action = new MyAction(newButton);
                action->setText(QString::number(x));
                action->setData(x);

                newButton->setDefaultAction(action);
            }

            Q_ASSERT(newButton);
            QObject::connect(newButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);
            _toolBar->addWidget(newButton);
        }
    }

    if (_currentPage < _totalPages)
    {
        QToolButton* nextButton = new MyToolButton(this);
        QObject::connect(nextButton, &QToolButton::triggered, this, &PaginationWidget::onButtonClicked);

        nextButton->setDefaultAction(new MyAction(tr("Next"), nextButton));
        nextButton->defaultAction()->setData(_currentPage + 1);
        _toolBar->addWidget(nextButton);
    }
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

    _toolBar = new QToolBar(this);
    _toolBar->setStyleSheet(strPaginationWidgetStyle);
    _buttonLayout->addWidget(_toolBar);

    createPreviousButtons();

    // add the anchor
    QToolButton* anchorButton = new MyToolButton(_toolBar);
    anchorButton->setText(QString::number(_currentPage));
    anchorButton->setObjectName("currentPage");
    _toolBar->addWidget(anchorButton);

    // add the buttons ot the right of the anchor (if there are any)
    createNextButtons();
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