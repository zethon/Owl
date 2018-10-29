#include "Data/Board.h"
#include "Data/BoardManager.h"
#include <Parsers/Forum.h>
#include "BoardUpdateWorker.h"

namespace owl
{

class UpdaterMutexTryLocker
{
    QMutex&		_m;
    bool		_locked;

public:
    UpdaterMutexTryLocker(QMutex &m)
        : _m(m),
        _locked(m.tryLock())
    {
        // do nothing
    }

    virtual ~UpdaterMutexTryLocker()
    {
        if (_locked)
        {
            _m.unlock();
        }
    }

    UpdaterMutexTryLocker(const UpdaterMutexTryLocker&) = delete;

    bool isLocked() const
    {
        return _locked;
    }
};

BoardUpdateWorker::BoardUpdateWorker(BoardPtr board)
    : _board(board)
{
    // do nothing
}

void BoardUpdateWorker::doWork()
{
    if (_isDeleted)
    {
        return;
    }

    UpdaterMutexTryLocker locker(_mutex);

    if (!locker.isLocked())
    {
        logger()->trace("Updater for board %1(%2) is running. Skipping this round...",
                        _board->getName(), _board->getDBId());

        return;
    }

    if (_board != nullptr)
    {
        logger()->debug("doWork() for board '%1'", _board->getName());

        try
        {
            _board->updateUnread();
            checkStructureUpdate();
        }
        catch (const WebException& ex)
        {
            logger()->error("Error during BoardUpdateWorker::doWork(): %1", ex.what());
        }
    }

    QTimer::singleShot(1000 * _board->getOptions()->get<std::uint32_t>("refreshRate"), this, SLOT(doWork()));
}

void BoardUpdateWorker::checkStructureUpdate()
{
    if (_isDeleted)
    {
        return;
    }

    // how long between each structure check (in seconds)
    const uint iRefreshPeriod = 60 * 60 * 24; // one day

    QDateTime boardTime = _board->getLastUpdate();
    if (logger()->isTraceEnabled())
    {
        logger()->trace("Board %1(%2) - last update was %3",_board->getName(), _board->getDBId(), boardTime);
    }

    if (boardTime.secsTo(QDateTime::currentDateTime()) >= iRefreshPeriod)
    {
        logger()->debug("Board %1(%2) - verifying forum structure", _board->getName(), _board->getDBId());

        BoardPtr savedBoard = BOARDMANAGER->loadBoard(_board->getDBId());
        ForumPtr savedRoot = savedBoard->getRoot();

        if (savedRoot != nullptr)
        {
            // update the Board::lastUpdate member
            ForumPtr root = _board->getRootStructure(false);
            if (root != nullptr)
            {
                if (_board->getRoot()->isStructureEqual(root))
                {
                    logger()->trace("Board %1(%2) - stored structure and online structure are the same", _board->getName(), _board->getDBId());
                }
                else
                {
                    logger()->trace("Board %1(%2) - stored structure and online structure are NOT the same", _board->getName(), _board->getDBId());
                    Q_EMIT onForumStructureChanged(_board);
                }

                _board->setLastUpdate(QDateTime::currentDateTime());
                BOARDMANAGER->updateBoard(_board);
            }
            else if (logger()->isWarnEnabled())
            {
                logger()->warn("Board %1(%2) - getRootStructure() returned a 'nullptr' root", _board->getName(), _board->getDBId());
            }
        }
        else if (logger()->isWarnEnabled())
        {
            logger()->warn("Board %1(%2) - loadBoard(), getRoot() returned a 'nullptr' root", _board->getName(), _board->getDBId());
        }
    }
}

}
