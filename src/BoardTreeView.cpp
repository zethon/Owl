#include <Utils/Settings.h>
#include "Data/BoardManager.h"
#include "BoardTreeView.h"

namespace owl
{

const QString gTreeStyle(R"(
QTreeView
{
    show-decoration-selected: 1;
    background-color: ${boardlist.background.color};
    color: ${boardlist.text.color};
    font-size: 12pt;
    font-family: "Helvetica, sans-serif";
}

QTreeView::item:selected
{
    color: ${boardlist.text.color};
    background-color: #567dbc;
}

)");

BoardTreeView::BoardTreeView(QWidget* parent)
	: QTreeView(parent)
{
    _noBoardsConfigured = new QLabel("No boards configured.<br/><a href=\"owl:///boardconfig\" style=\"color:lightblue;\">Click here</a> to configure.", this);
	_noBoardsConfigured->setFont(QFont("Verdana", 14, QFont::Bold, false));
    _noBoardsConfigured->setStyleSheet("color: lightgray;");
	_noBoardsConfigured->setVisible(BoardManager::instance()->getBoardCount() <= 0);

	QObject::connect(_noBoardsConfigured, 
		SIGNAL(linkActivated(const QString&)), 
		this, 
		SIGNAL(linkActivated(const QString&)));

	QTimer* timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, [this]()
	{
        loadStyleSettings();
        setStyleSheet(createStyle());
		_noBoardsConfigured->move(rect().center() - _noBoardsConfigured->rect().center());
	});

	timer->setSingleShot(true);
	timer->start();
}

void BoardTreeView::setModel(QAbstractItemModel * model)
{
	QTreeView::setModel(model);
	_boardModel = qobject_cast<BoardsModel*>(model);
}

void BoardTreeView::markForumRead(ForumPtr f)
{
    assert(f && f->getBoard().lock());
	assert(_boardModel);

	auto item = _boardModel->getForumItem(f);
	
	QFont itemFont(item->font());
	itemFont.setBold(false);

	item->setIcon(QIcon(":/icons/forum.png"));
	item->setFont(itemFont);
}

void BoardTreeView::selectOneBelow(QModelIndex index)
{
	if (index.isValid()) // && !isExpanded(index))
	{
		expandIt(index);

		QModelIndex nextIndex = indexBelow(index);
		if (nextIndex.isValid())
		{
			QVariant var = model()->data(nextIndex, Qt::UserRole + 1);

			if (var.canConvert<ForumPtr>())
			{
				this->setCurrentIndex(nextIndex);
			}
		}
	}
}
    
void BoardTreeView::selectForumBelow(const QModelIndex& index)
{
    if (index.isValid())
    {
        expandIt(index);
        
        QModelIndex next = indexBelow(index);
        
        if (next.data(Qt::UserRole+1).canConvert<ForumPtr>())
        {
            auto board = next.data(Qt::UserRole+1).value<ForumPtr>();
            if (board->getForumType() == Forum::FORUM)
            {
                setCurrentIndex(next);
            }
        }
    }
}

void BoardTreeView::expandIt(QModelIndex myIndex)
{
	if (myIndex.isValid())
	{
		expand(myIndex);
	}

	QModelIndex nextIndex = indexBelow(myIndex);
	while (nextIndex.isValid())
	{
		if (myIndex.parent() == nextIndex.parent())
		{
			break;
		}

		expand(nextIndex);
		nextIndex = indexBelow(nextIndex);
	}
}
    
void BoardTreeView::resizeEvent(QResizeEvent* event)
{
    const int widthCutOff = 100;
    
    QTreeView::resizeEvent(event);
    
    if (event->size().width() <= widthCutOff && event->oldSize().width() > widthCutOff)
    {
        this->setWordWrap(false);
    }
    else if (event->size().width() > widthCutOff && event->oldSize().width() <= widthCutOff)
    {
        this->setWordWrap(true);
    }

	_noBoardsConfigured->move(rect().center() - _noBoardsConfigured->rect().center());
}

void BoardTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	QTreeView::currentChanged(current, previous);

	if (_boardModel)
	{
		auto prevItem = _boardModel->itemFromIndex(previous);
		if (prevItem && prevItem->data().canConvert<ForumPtr>())
		{
			auto forum = prevItem->data().value<ForumPtr>();
			forum->getThreads().clear();
		}
	}
}

void BoardTreeView::setHasBoards(bool bHasBoards)
{
	if (bHasBoards)
	{
		_noBoardsConfigured->hide();
	}
	else
	{
		_noBoardsConfigured->show();
	}
}

void BoardTreeView::reloadView()
{
    loadStyleSettings();
    setStyleSheet(createStyle());
}

// creates a style for the entire widget according to the app settings
QString BoardTreeView::createStyle()
{
    QString treeStyle { gTreeStyle };

    QStringList settingList;
    settingList << "boardlist.background.color" << "boardlist.highlight.color" << "boardlist.icons.visible" << "boardlist.text.color";
    for (const auto& setting : settingList)
    {
        treeStyle.replace(QString("${%1}").arg(setting), SettingsObject().read(setting).toString());
    }

    return treeStyle;
}

// modifies each board's boardItem.html according to the app settings
void BoardTreeView::loadStyleSettings()
{
	if (_boardModel)
	{
		const SettingsObject appSettings;

		QStringList settingList;
		settingList << "boardlist.text.color" << "boardlist.highlight.color";

		const auto rowCount = _boardModel->rowCount();
		for (int row = 0; row < rowCount; row++)
		{
			auto item = _boardModel->item(row);
			if (item && item->data(BOARDITEMPTR_ROLE).canConvert<BoardWeakPtr>())
			{
				BoardWeakPtr bw = item->data(BOARDITEMPTR_ROLE).value<BoardWeakPtr>();
				BoardPtr board = bw.lock();
				if (board)
				{
					auto& doc = board->getBoardItemDocument();
					for (const auto& setting : settingList)
					{
						doc->setOrAddVar(QString("${%1}").arg(setting), appSettings.read(setting).toString());
					}

					// Hack! Since Qt's limited HTML/CSS support does not support a "visible" or "display" attribute
					// we have to hack the board icon by commenting out the <tr> element that contains it
					if (!appSettings.read("boardlist.icons.visible").toBool())
					{
						doc->setOrAddVar("%BOARDICONSHOWSTART%", "<!--");
						doc->setOrAddVar("%BOARDICONSHOWEND%", "-->");
					}
					else
					{
						doc->setOrAddVar("%BOARDICONSHOWSTART%", "");
						doc->setOrAddVar("%BOARDICONSHOWEND%", "");
					}


					doc->reloadHtml();
				}
			}
		}
	}
}

} // namespace
