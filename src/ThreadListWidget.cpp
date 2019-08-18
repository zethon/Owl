#include <QQmlContext>
#include <QQuickItem>
#include <Utils/Settings.h>
#include "Data/Board.h"
#include "ThreadListWidget.h"

namespace owl
{

ThreadListWidget::ThreadListWidget(QWidget* parent/* = nullptr*/)
    : QQuickWidget(parent)
{
    SettingsObject settings;
    _dtOptions.useDefault = settings.read("datetime.format").toString() == "default";
    _dtOptions.usePretty = settings.read("datetime.date.pretty").toBool();
    _dtOptions.dateFormat = settings.read("datetime.date.format").toString();
    _dtOptions.timeFormat = settings.read("datetime.time.format").toString();

    setFocusPolicy(Qt::TabFocus);
    setResizeMode(QQuickWidget::SizeRootObjectToView);

    QQmlContext* root = rootContext();
    root = this->rootContext();
    root->setContextProperty("threadListPage", this);
    root->setContextProperty("threadListModel", QVariant{});

    setSource(QUrl("qrc:/qml/threadList.qml"));
}

ThreadListWidget::~ThreadListWidget()
{
    clearList();
}

void ThreadListWidget::setThreadList(const ThreadList& threadList)
{
    if (rootContext())
    {
        clearList();

        QList<QObject*> modelList;
        for (const auto& t : threadList)
        {
            std::shared_ptr<ThreadObject> obj = std::make_shared<ThreadObject>(t, _dtOptions);
            _threadList.push_back(obj);

            if (!t->isSticky() || _showStickies)
            {
                modelList.push_back(obj.get());
            }

            QObject::connect(obj.get(), &ThreadObject::threadLoading,[this, t]()
            {
                _currentThread = t;
                Q_EMIT this->threadLoading();
            });
        }

        rootContext()->setContextProperty("threadListModel", QVariant::fromValue(modelList));
        QMetaObject::invokeMethod(rootObject(), "setHasThreads", Qt::DirectConnection, Q_ARG(QVariant, (bool)(_threadList.size() > 0)));
    }
}

void ThreadListWidget::clearList()
{
    // in order to clean the previously allocated ThreadObject's without
    // screwing up QML's internal list, we need to reset the QML property
    // and only after that clear our managed list
    if (rootContext())
    {
        rootContext()->setContextProperty("threadListModel", QVariant{});
    }

    std::for_each(_threadList.begin(), _threadList.end(),[](ThreadObjectPtr t) { t->disconnect(); });
    _threadList.clear();
}

void ThreadListWidget::resetView()
{
    clearList();

    // reset the view port
    QMetaObject::invokeMethod(rootObject(), "setHasThreads", Qt::DirectConnection, Q_ARG(QVariant, (bool)(_threadList.size() > 0)));
}

void ThreadListWidget::reload()
{
    SettingsObject settings;
    _dtOptions.useDefault = settings.read("datetime.format").toString() == "default";
    _dtOptions.usePretty = settings.read("datetime.date.pretty").toBool();
    _dtOptions.dateFormat = settings.read("datetime.date.format").toString();
    _dtOptions.timeFormat = settings.read("datetime.time.format").toString();

    refreshThreadDisplay();
}

std::weak_ptr<Thread> ThreadListWidget::getCurrentThread() const
{
    return _currentThread;
}

void ThreadListWidget::refreshThreadDisplay()
{
    if (rootContext())
    {
        QList<QObject*> modelList;

        for (std::shared_ptr<ThreadObject> top : _threadList)
        {
            if (!top->sticky() || _showStickies)
            {
                modelList.push_back(top.get());
            }
        }

        rootContext()->setContextProperty("threadListModel", QVariant::fromValue(modelList));
        QMetaObject::invokeMethod(rootObject(), "setHasThreads", Qt::DirectConnection,
            Q_ARG(QVariant, static_cast<bool>(_threadList.size() > 0)));
    }
}

void ThreadListWidget::loadInBrowser(std::int32_t index)
{
    if (index < _threadList.size())
    {
        ThreadPtr thread = _threadList[index]->getSharedPtr();
        if (thread)
        {
            BoardPtr board = thread->getBoard().lock();
            if (board)
            {
                const QString url = board->getParser()->getItemUrl(thread);
                QDesktopServices::openUrl(url);
            }
        }
    }
}

void ThreadListWidget::copyUrl(std::int32_t index)
{
    if (index < _threadList.size())
    {
        ThreadPtr thread = _threadList[index]->getSharedPtr();
        if (thread)
        {
            BoardPtr board = thread->getBoard().lock();
            if (board)
            {
                const QString url = board->getParser()->getItemUrl(thread);
                qApp->clipboard()->setText(url);
            }
        }
    }
}

void ThreadObject::loadThread()
{
    ThreadPtr thread = _threadPtr.lock();
    if (thread)
    {
        auto board = thread->getBoard().lock();
        if (board)
        {
            board->setCurrentThread(thread);
            board->requestPostList(thread);
            threadLoading();
        }
    }
}

ThreadObject::ThreadObject(ThreadPtr ptr, const DateTimeFormatOptions& dtOptions, QObject *parent)
    : QObject(parent),
      _threadID(ptr->getId()),
      _title(ptr->getTitle()),
      _author(ptr->getAuthor()),
      _iconUrl(ptr->getIconUrl()),
      _replyCount(ptr->getReplyCount()),
      _unread(ptr->hasUnread()),
      _sticky(ptr->isSticky()),
      _threadPtr(ptr),
      _dtOptions(dtOptions)
{
    const auto lastPost = ptr->getLastPost();
    if (lastPost)
    {
        _lastAuthor.append(lastPost->getAuthor());

    }
}

QString ThreadObject::previewText() const
{
    QString retval;
    const ThreadPtr thread = _threadPtr.lock();

    if (thread)
    {
        retval = thread->getPreviewText();
    }

    return retval;
}

QString ThreadObject::createdTimeText() const
{
    QString dateText = _dateText; // fail safe
    const ThreadPtr thread = _threadPtr.lock();
    if (thread)
    {
        const auto postList = thread->getPosts();
        if (!postList.empty())
        {
            dateText = postList.front()->getPrettyTimestamp(_dtOptions);
        }
    }
    return dateText;
}

QString ThreadObject::dateText() const
{
    QString dateText = _dateText; // fail safe
    const ThreadPtr thread = _threadPtr.lock();
    if (thread)
    {
        const auto lastPost = thread->getLastPost();
        if (lastPost)
        {
            dateText = lastPost->getPrettyTimestamp(_dtOptions);
        }
    }
    return dateText;
}

} // namespace
