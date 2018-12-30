#pragma once

#include <QWidget>
#include <QWidgetAction>

class QToolBar;
class QToolButton;

namespace owl
{

class GotoPageWidget : public QWidget
{
    Q_OBJECT

public:
    ~GotoPageWidget() = default;
    explicit GotoPageWidget(QWidget* parent);
};

// https://stackoverflow.com/questions/27014845/how-can-i-add-custom-widget-as-popup-menu-for-toolbutton
class GotoPageWidgetAction : public QWidgetAction
{
    Q_OBJECT

public:
    ~GotoPageWidgetAction() = default;
    explicit GotoPageWidgetAction(QWidget* parent);

protected:
    QWidget* createWidget(QWidget* parent);
};

class PaginationWidget : public QWidget
{
    Q_OBJECT

public:
    ~PaginationWidget() = default;
    PaginationWidget(QWidget* parent = nullptr);

    std::uint32_t current() const { return _currentPage; }
    std::uint32_t total() const { return _totalPages; }
    void setPages(std::uint32_t current, std::uint32_t total);

Q_SIGNALS:
    void doGotoPage(std::uint32_t);

private:
    void setPrevButtons();
    void setNextButtons();

    std::uint32_t   _currentPage;
    std::uint32_t   _totalPages;

    QToolBar*       _toolBar;
    QAction*        _prevAction;
    QAction*        _nextAction;

    QList<QAction*> _actionList;
};

} // namespace
