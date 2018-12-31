#pragma once

#include <QWidget>
#include <QWidgetAction>

class QToolBar;
class QToolButton;
class QHBoxLayout;

namespace owl
{

class GotoPageWidget : public QWidget
{
    Q_OBJECT

public:
    ~GotoPageWidget() = default;
    explicit GotoPageWidget(std::uint32_t totalPages, QWidget* parent);

Q_SIGNALS:
    void gotoPage(std::uint32_t pageNumber);
};

// https://stackoverflow.com/questions/27014845/how-can-i-add-custom-widget-as-popup-menu-for-toolbutton
class GotoPageWidgetAction : public QWidgetAction
{
    Q_OBJECT

public:
    ~GotoPageWidgetAction() = default;
    explicit GotoPageWidgetAction(std::uint32_t totalPages, QWidget* parent);

Q_SIGNALS:
    void gotoPage(std::uint32_t pageNumber);

protected:
    QWidget* createWidget(QWidget* parent);

private:
    std::uint32_t   _totalPages;
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

private Q_SLOTS:
    void onButtonClicked(QAction*);

private:
    void createPreviousButtons();
    void createNextButtons();

    std::uint32_t   _currentPage;
    std::uint32_t   _totalPages;

    QHBoxLayout*    _buttonLayout;
    QToolBar*       _toolBar = nullptr;
};

} // namespace
