#pragma once
#include <memory>
#include <QObject>
#include <QMutex>

namespace spdlog
{
    class logger;
}

namespace owl
{

class Board;
using BoardPtr = std::shared_ptr<Board>;
using BoardWeakPtr = std::weak_ptr<Board>;

class BoardUpdateWorker : public QObject
{
	Q_OBJECT

public:

    BoardUpdateWorker(BoardPtr board);

    virtual ~BoardUpdateWorker() = default;
    
    void setIsDone(bool var) { _isDeleted = var; }
    
Q_SIGNALS:
    void onForumStructureChanged(BoardPtr board);

protected Q_SLOTS:
    void doWork();
//    void checkStructureUpdate();

private:

    BoardWeakPtr    _board;
    QMutex          _mutex;
    bool            _isDeleted = false;

    std::shared_ptr<spdlog::logger>  _logger;
};

} // namespace
