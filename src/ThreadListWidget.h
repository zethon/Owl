#pragma once
#include <QQuickWidget>
#include <Parsers/Forum.h>
#include <Utils/DateTimeParser.h>

namespace owl
{

struct DateTimeFormatOptions;

class ThreadObject final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString threadID READ threadID WRITE setThreadID NOTIFY threadIDChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString previewText READ previewText NOTIFY previewTextChanged)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
    Q_PROPERTY(QString lastAuthor READ lastAuthor WRITE setLastAuthor NOTIFY lastAuthorChanged)
    Q_PROPERTY(QString dateText READ dateText WRITE setDateText NOTIFY dateTextChanged)
    Q_PROPERTY(QString iconUrl READ iconUrl WRITE setIconUrl NOTIFY iconUrlChanged)
    Q_PROPERTY(uint replyCount READ replyCount WRITE setReplyCount NOTIFY replyCountChanged)
    Q_PROPERTY(bool unread READ unread WRITE setUnread NOTIFY unreadChanged)
    Q_PROPERTY(bool sticky READ sticky WRITE setSticky NOTIFY stickyChanged)

public:
    explicit ThreadObject(ThreadPtr ptr, const DateTimeFormatOptions& dtOptions, QObject* parent = 0);
    virtual ~ThreadObject() = default;

    QString threadID() const { return _threadID; }
    void setThreadID(const QString& var) { _threadID = var; }

    QString title() const { return _title; }
    void setTitle(const QString& var) { _title = var; }

    QString previewText() const;

    QString author() const { return _author; }
    void setAuthor(const QString& var) { _author = var; }

    QString lastAuthor() const { return _lastAuthor; }
    void setLastAuthor(const QString& var) { _lastAuthor = var; }

    QString dateText() const;
    void setDateText(const QString& var) { _dateText = var; }

    QString iconUrl() const { return _iconUrl; }
    void setIconUrl(const QString& var) { _iconUrl = var; }

    uint replyCount() const { return _replyCount; }
    void setReplyCount(bool var) { _replyCount = var; }

    bool unread() const { return _unread; }
    void setUnread(bool var) { _unread = var; }

    bool sticky() const { return _sticky; }
    void setSticky(bool var) { _sticky = var; }

    ThreadPtr getSharedPtr() { return _threadPtr.lock(); }

    Q_INVOKABLE void loadThread();

Q_SIGNALS:
    void threadIDChanged();
    void titleChanged();
    void previewTextChanged();
    void authorChanged();
    void lastAuthorChanged();
    void dateTextChanged();
    void iconUrlChanged();
    void replyCountChanged();
    void unreadChanged();
    void stickyChanged();

    // sent when the thread is clicked on and a request to load it has been sent
    void threadLoading();

private:
    QString         _threadID;
    QString         _title;
    QString         _author;
    QString         _lastAuthor;
    QString         _dateText;
    QString         _iconUrl;
    std::uint32_t   _replyCount = 0;
    bool            _unread = false;
    bool            _sticky = false;
    bool            _selected = false;

    QString _name;
    QString _color;

    std::weak_ptr<owl::Thread>  _threadPtr;
    const DateTimeFormatOptions&      _dtOptions;
};

using ThreadObjectPtr = std::shared_ptr<ThreadObject>;

// C++ & QML Models: http://doc.qt.io/qt-5/qtquick-modelviewsdata-cppmodels.html
class ThreadListWidget : public QQuickWidget
{
    Q_OBJECT

    Q_PROPERTY(bool showStickies READ showStickies WRITE setShowStickies NOTIFY showStickiesChanged)

public:
    ThreadListWidget(QWidget* parent = nullptr);
    virtual ~ThreadListWidget();

    void setShowStickies(bool v) { _showStickies = v; }
    bool showStickies() { return _showStickies; }

    void setThreadList(const ThreadList& threadList);
    void clearList(); // clears the list in C++ and QML
    void resetView(); // clears the list and the view
    void reload(); // reloads changeable settings and updates display

    std::weak_ptr<owl::Thread> getCurrentThread() const;

    // will refresh the UI display, mainly used when toggling show-stickies on and off
    Q_INVOKABLE void refreshThreadDisplay();

    // called from
    Q_INVOKABLE void loadInBrowser(std::int32_t index);
    Q_INVOKABLE void copyUrl(std::int32_t index);

Q_SIGNALS:
    void threadLoading();
    void showStickiesChanged();

private:
    QList<ThreadObjectPtr>      _threadList;
    std::weak_ptr<owl::Thread>  _currentThread;
    bool                        _showStickies = true;
    DateTimeFormatOptions       _dtOptions;
};

} // namespace
