// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QSqlDatabase>
#include <QString>
#include <log4qt/logger.h>
#include <Utils/Exception.h>
#include <Utils/Rijndael.h>
#include "Board.h"

#define MAX_BOARDS                  32
#define DBPASSWORD_SEED             "OwlPasswordSeed"
#define DBPASSWORD_KEY              "OwlPasswordKey"
#define OWL_DATABASE_NAME           "OwlDB"

#define BOARDMANAGER                BoardManager::instance()

namespace owl
{

class BoardManager;
using BoardManagerPtr = std::shared_ptr<BoardManager>;

class BoardManager : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
    
public:
	static BoardManagerPtr instance()
	{
		if (!_instance.get())
		{
			_instance.reset(new BoardManager());
		}

		return _instance;
	}

    BoardManager (const BoardManager&) = delete;
	virtual ~BoardManager(); 

	size_t getBoardCount() const;
    
    void init();
	void reload();

    
    // sorts the _boardList according to the board's displayOrder option
    void sort();

    void firstTimeInit();
    
    const BoardList& getBoardList() const { return _boardList; }

	// CRUD
	bool createBoard(BoardPtr board);

	// returns a shared ptr to the board object loaded
	// if the board doesn't exist, then the boardId = -1
	BoardPtr loadBoard(int boardId);

	uint updateBoards();
	bool updateBoard(BoardPtr board);
	void updateBoardOptions(BoardPtr b, bool bDoCommit = false);

	bool deleteBoard(BoardPtr board);

	BoardPtr boardByItem(QStandardItem* item) const;
    
    // FORUM - CRUD
    bool deleteForumVars(const QString& forumId) const;
    
protected:
  	BoardManager();

private:
	void createBoardOptions(BoardPtr board);
	
	void createForumEntries(ForumPtr forum, BoardPtr board);
	void createForumVars(ForumPtr forum);

	void retrieveSubForumVars(ForumPtr forum);
	void retrieveSubForumList(BoardPtr board, ForumPtr forum, bool bDeep = false);
	bool retrieveBoardForums(BoardPtr b);
    
    void loadBoardOptions(const BoardPtr& b);

	static bool boardDisplayOrderLessThan(BoardPtr b1, BoardPtr b2)
	{
		uint iB1DisplayOrder = b1->getOptions()->get<std::uint32_t>("displayOrder");
		uint iB2DisplayOrder = b2->getOptions()->get<std::uint32_t>("displayOrder");

		return iB1DisplayOrder < iB2DisplayOrder;
	}

	QSqlDatabase _db;
	CRijndaelPtr _encryptor;

    BoardList _boardList;

	QMutex _mutex;

	static BoardManagerPtr _instance;
};

class BoardManagerException : public OwlException
{
public:
    BoardManagerException(const QString& msg, const QString& query)
        : OwlException(msg),
		  _query(query)
	{
	}

    BoardManagerException(const QString& msg)
        : OwlException(msg)
	{
	}

	virtual ~BoardManagerException() throw()
	{
	}

	const QString& query() const { return _query; }

private:
	QString _query;
};

} // namespace owl
