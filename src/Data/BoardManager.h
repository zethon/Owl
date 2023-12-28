// Owl - www.owlclient.com
// Copyright (c) 2012-2023, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QSqlDatabase>
#include <QString>
#include <Utils/Exception.h>
#include "Board.h"

#define MAX_BOARDS                  32
#define DBPASSWORD_SEED             "OwlPasswordSeed"
#define DBPASSWORD_KEY              "OwlPasswordKey"
#define OWL_DATABASE_NAME           "OwlDB"

#define BOARDMANAGER                BoardManager::instance()

namespace spdlog
{
    class logger;
}

namespace owl
{

class BoardManager;
using BoardManagerPtr = std::shared_ptr<BoardManager>;

class BoardManager final
    : public QObject
{
    Q_OBJECT
    
public:
	static BoardManagerPtr instance()
	{
		if (!_instance.get())
		{
			_instance.reset(new BoardManager());
		}

		return _instance;
	}

    virtual ~BoardManager() = default;
    BoardManager (const BoardManager&) = delete;
	
    QSqlDatabase initializeDatabase(const QString& filename);
    
    void loadBoards(bool resetdb);
    void reload();

    std::size_t getBoardCount() const;
    const BoardList& getBoardList() const { return _boardList; }

	// CRUD
	bool createBoard(BoardPtr board);

	// returns a shared ptr to the board object loaded
	// if the board doesn't exist, then the boardId = -1
	BoardPtr getBoardInfo(int boardId);

	uint updateBoards();
	bool updateBoard(BoardPtr board);
	void updateBoardOptions(BoardPtr b, bool bDoCommit = false);

	bool deleteBoard(BoardPtr board);

    BoardPtr boardByIndex(std::size_t index) const;
    BoardPtr boardByUUID(const std::string& uid) const;
    
    // FORUM - CRUD
    bool deleteForumVars(const QString& forumId) const;

Q_SIGNALS:
    void onBeginAddBoard(int index);
    void onEndAddBoard();
    void onBeginRemoveBoard(int index);
    void onEndRemoveBoard();


private:
    BoardManager();

    QSqlDatabase getDatabase(bool doOpen = true) const;

	void createBoardOptions(BoardPtr board);	
	void createForumEntries(ForumPtr forum, BoardPtr board);
	void createForumVars(ForumPtr forum);

	void retrieveSubForumVars(ForumPtr forum);
	void retrieveSubForumList(BoardPtr board, ForumPtr forum, bool bDeep = false);
	bool retrieveBoardForums(BoardPtr b);
    
    void loadBoardOptions(const BoardPtr& b);

    // sorts the _boardList according to the board's displayOrder option
    void sort();

	static bool boardDisplayOrderLessThan(BoardPtr b1, BoardPtr b2)
	{
		uint iB1DisplayOrder = b1->getOptions()->get<std::uint32_t>("displayOrder");
		uint iB2DisplayOrder = b2->getOptions()->get<std::uint32_t>("displayOrder");

		return iB1DisplayOrder < iB2DisplayOrder;
	}

    static BoardManagerPtr _instance;

    BoardList _boardList;
	QMutex _mutex;
    
    std::string                         _databaseFilename;
    std::shared_ptr<spdlog::logger>     _logger;
};

} // namespace owl
