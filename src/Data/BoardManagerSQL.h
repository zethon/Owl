namespace owl
{

const char* createDatabaseSQLString = R"SQL(

-- NOTE: Because of limitations in Qt's SQLite implementation, SQL query strings can only contain on
-- statement. Multiple statements in a single query string will cause the query to fail. When Owl 
-- creates a blank database, it seperates each statement in this file by a semicolon. Hence, indidvidual
-- statements CANNOT contain semicolons.

BEGIN TRANSACTION;

CREATE TABLE boards                
(                                    
	boardid INTEGER PRIMARY KEY,
	enabled INTEGER,
	autologin INTEGER,
	name TEXT,                           
	url TEXT,                            
	parser TEXT,                         
	serviceUrl TEXT,
	username TEXT,                       
	password TEXT,                       
	icon BLOB,	
	lastupdate TEXT			-- last time the forums of the board were synced
);

CREATE TABLE boardvars 
(
	boardvarid INTEGER PRIMARY KEY,
	boardid NUMERIC, 
	name TEXT, 
	value TEXT
);

CREATE TABLE forums
(
	id INTEGER PRIMARY KEY,	-- PK for the row
	boardId INTEGER,		-- FK to the boards.boardid
	forumId TEXT, 			-- forumId on the board
	parentId TEXT, 			-- FK to the parent forumId
	forumName TEXT,			-- dislay name of the forum
	forumType TEXT, 		-- enum, FORUM, CATEGORY, LINK
	forumOrder INTEGER		-- display order of the forum
);

CREATE TABLE forumvars 
(
	forumvarid INTEGER PRIMARY KEY,	
	forumsid NUMERIC, 					-- PK to forums.id
	name TEXT, 							-- key of the 	
	value TEXT
);

CREATE TABLE style
(
	styleid INTEGER PRIMARY KEY,	-- PK for the row
	name TEXT
);

CREATE TABLE template
(
	templateid INTEGER PRIMARY KEY,
	styleid INTEGER,
	name TEXT,
	template TEXT	
);

COMMIT;

)SQL";

}