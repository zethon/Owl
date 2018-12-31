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
    Q_ASSERT(current > 0 && current <= total);

    _currentPage = current;
    _totalPages = total;

    _toolBar->clear();

    _prevButton->setVisible(_currentPage > 1);
    _prevButton->defaultAction()->setData(_currentPage - 1);
    _toolBar->addWidget(_prevButton);

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
