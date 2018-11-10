#include "BoardsModel.h"
#include <spdlog/spdlog.h>

namespace owl
{

/////////////////////////////////////////////////////////////////////////
// BoardItemDelegate
/////////////////////////////////////////////////////////////////////////
void BoardItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    auto data = index.data(BOARDITEMPTR_ROLE);

    if (data.canConvert<BoardWeakPtr>())
    {
        const BoardPtr board = data.value<BoardWeakPtr>().lock();

        if (board)
        {
            QStyleOptionViewItem options = option;
            initStyleOption(&options, index);

            painter->save();

            /* Call this to get the focus rect and selection background. */
            options.text = "";
            options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

            /* Draw using our rich text document. */
            painter->translate(options.rect.left(), options.rect.top());
            QRect clip(0, 0, options.rect.width(), options.rect.height());
            board->getBoardItemDocument()->drawContents(painter, clip);

            painter->restore();
            return;
        }
    }
    else if (index.data(LASTITEM_ROLE).toBool())
    {
        // the LASTITEM_ROLE object gets a background color that is drawn at the bottom
        // of each board's tree entry
        painter->save();
        painter->fillRect(option.rect, QBrush(QColor(90, 90, 90)));
        painter->restore();
        return;
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QSize BoardItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    auto data = index.data(BOARDITEMPTR_ROLE);

    if (data.canConvert<BoardWeakPtr>())
    {
        auto board = data.value<BoardWeakPtr>().lock();
        if (board)
        {
            auto boardDoc = board->getBoardItemDocument();
            QStyleOptionViewItem options = option;
            initStyleOption(&options, index);

            boardDoc->setTextWidth(300);
            return QSize(boardDoc->idealWidth(), boardDoc->size().height());
        }
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

/////////////////////////////////////////////////////////////////////////
// BoardsModel
/////////////////////////////////////////////////////////////////////////

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
const uint BoardsModel::ITEMHEIGHT = 35;
#else
const uint BoardsModel::ITEMHEIGHT = 24;
#endif

BoardsModel::BoardsModel(QWidget* parent)
    : QStandardItemModel(parent)
{
    // do nothing
}

BoardsModel::~BoardsModel()
{
    // do nothing
}

/// Seaches the model for Board with the same name and username. If not found
/// then the board is added to the model.
/**
    \param BoardPtr Board to be added to the model.

    \return Pointer to the QStandardItem added or NULL if it already existed.

    \TODO: this method should search by the Board's *URL* and not *name*
*/
QStandardItem* BoardsModel::addBoardItem(const BoardPtr& b, bool bThrowOnFail)
{
    QStandardItem* retItem(NULL);
    QStandardItem* boardItem = getBoardItem(b, false);
    bool doCreate = false;

    if (boardItem == NULL)
    {
        doCreate = true;
    }
    else
    {
        BoardPtr board = boardItem->data().value<BoardWeakPtr>().lock();

        if (board->getUsername() == b->getUsername())
        {
            if (bThrowOnFail)
            {
                OWL_THROW_EXCEPTION(OwlException(
                    QString("Board '%1' with username '%2' already exists in model")
                        .arg(b->getName())
                        .arg(b->getUsername()))
                );
            }
            else
            {
                retItem = boardItem;
            }
        }
        else
        {
            doCreate = true;
        }
    }

    if (doCreate)
    {
        int iRowCount = this->rowCount();

        try
        {
            QByteArray buffer(b->getFavIcon().toLatin1());
            QImage image = QImage::fromData(QByteArray::fromBase64(buffer));
            QIcon icon(QPixmap::fromImage(image));

            retItem = new QStandardItem(b->getName());
            retItem->setData(QVariant::fromValue(BoardWeakPtr(b)), BOARDITEMPTR_ROLE);
            retItem->setData(b->getUrl(), Qt::ToolTipRole);

            insertRow(iRowCount, retItem);

            for (ForumPtr forum : b->getRoot()->getForums())
            {
                addForums(b, forum);
            }

            // insert a last item object so that we can properly paint the servcesTree
            QStandardItem* lastItem(new QStandardItem());
            lastItem->setData((bool)true, LASTITEM_ROLE);
            lastItem->setSizeHint(QSize(lastItem->sizeHint().width(), 1));
            appendRow(lastItem);
        }
        catch (const owl::OwlException& ex)
        {
            spdlog::get("Owl")->error("Failed to create parser of type '{}' for board '{}': {}",
                b->getProtocolName().toStdString(), b->getName().toStdString(), ex.what());

            throw;
        }
    }

    return retItem;
}

void BoardsModel::removeBoardItem(BoardPtr b)
{
    {
        // remove the entries from _index
        QMutexLocker locker(&_indexMutex);
        QString searchStr = QString("%1:").arg(b->getDBId());

        for (QString key : _index.keys())
        {
            if (key.contains(QRegularExpression("\\d\\:")))
            {
                _index.remove(key);
            }
        }
    }

    QStandardItem* bi = getBoardItem(b, false);

    if (bi != NULL)
    {
        auto lastItemIdx = index(bi->index().row() + 1, 0);
        auto itemLast = this->itemFromIndex(lastItemIdx);
        if (itemLast->data(LASTITEM_ROLE).toBool())
        {
            removeRow(lastItemIdx.row());
        }

        this->removeRow(bi->index().row());
    }
}

void BoardsModel::addForums(BoardPtr board, ForumPtr forum)
{
    QStandardItem*	boardItem(getBoardItem(board));
    QStandardItem*	parentItem(NULL);

    if (forum->IsRoot())
    {
        parentItem = boardItem;
    }
    else
    {
        QString forumKey = getIndexKey(board, forum->getParent());

        if (_index.contains(forumKey))
        {
            parentItem = _index.value(forumKey);
        }
        else if (forum->getParent() != NULL)
        {
            ForumPtr parent = forum->getParent()->upCast<ForumPtr>(false);

            if (parent->IsRoot())
            {
                parentItem = boardItem;
            }
        }
        else
        {
            OWL_THROW_EXCEPTION(OwlException("Cannot find parent item."));
        }
    }

    QStandardItem* subItem(NULL);
    QString subKey = getIndexKey(board, forum);

    if (_index.contains(subKey))
    {
        subItem = _index.value(subKey);
    }
    else
    {
        QTextDocument doc;
        doc.setHtml(forum->getName());

        subItem = new QStandardItem(doc.toPlainText());
        _index.insert(subKey, subItem);

        parentItem->appendRow(subItem);
    }

    if (forum->getForumType() == Forum::FORUM)
    {
        subItem->setIcon(QIcon(":/icons/forum.png"));
    }
    else if (forum->getForumType() == Forum::CATEGORY)
    {
        subItem->setIcon(QIcon(":/icons/category.png"));
        subItem->setFlags(Qt::ItemIsEnabled);
    }
    else if (forum->getForumType() == Forum::LINK)
    {
        subItem->setIcon(QIcon(":/icons/link.png"));
    }

    subItem->setData(QVariant::fromValue(forum));
    subItem->setSizeHint(QSize(subItem->sizeHint().width(), ITEMHEIGHT));
    forum->setModelItem(subItem);

    for (ForumPtr child : forum->getForums())
    {
        addForums(board, child);
    }
}

// updates the Foum and the Forum's children forums
QStandardItem* BoardsModel::updateForumItem(BoardPtr b, ForumPtr forum)
{
    QStandardItem* boardItem = getBoardItem(b);
    QStandardItem* parentItem(NULL);

    QString forumKey = getIndexKey(b, forum);

    if (forum->IsRoot())
    {
        parentItem = boardItem;
    }
    else
    {
        if (_index.contains(forumKey))
        {
            parentItem = _index.value(forumKey);
        }
        else
        {
            OWL_THROW_EXCEPTION(OwlException("Cannot find parent item."));
        }
    }

    QString strToolTip;

    for (ForumPtr f : forum->getForums())
    {
        QStandardItem* subItem(NULL);
        QString fKey = getIndexKey(b, f);

        if (_index.contains(fKey))
        {
            subItem = _index.value(fKey);
        }
        else
        {
            subItem = new QStandardItem(f->getName());
            _index.insert(fKey, subItem);

            parentItem->appendRow(subItem);
        }

        if (f->getForumType() == Forum::FORUM)
        {
            if (f->hasUnread())
            {
                subItem->setIcon(QIcon(":/icons/forum_new.png"));
            }
            else
            {
                subItem->setIcon(QIcon(":/icons/forum.png"));
            }

            strToolTip = QString("Last updated: %1").arg(f->getLastUpdated().toString());
            subItem->setToolTip(strToolTip);
            subItem->setData(QVariant::fromValue(f));
            subItem->setSizeHint(QSize(subItem->sizeHint().width(), ITEMHEIGHT));
        }
        else
        {
            // TODO: what is going on here?
//            logger()->error(QString("Unsupported forum type '%1/%2' with title '%3' on board '%4'")
//                .arg(f->getForumType()).arg(f->getForumTypeString()).arg(f->getName()).arg(b->getUrl()));
        }
    }

    return parentItem;
}

QStandardItem* BoardsModel::getBoardItem(BoardPtr board, bool bThrowOnFail /*= true*/)
{
    QStandardItem* ret(NULL);

    QList<QStandardItem*> items = findItems(board->getName(), Qt::MatchExactly);

    if (items.size() > 1)
    {
        for (QStandardItem* item : items)
        {
            BoardPtr b = item->data().value<BoardWeakPtr>().lock();

            if (b->getUsername() == board->getUsername())
            {
                ret = item;
                break;
            }
        }
    }
    else if (items.size() == 1)
    {
        ret = items.value(0);
    }
    else if (bThrowOnFail)
    {
        OWL_THROW_EXCEPTION(OwlException("Board's QStandardItem not found."));
    }

    return ret;
}

/**
 * @brief BoardsModel::getIndexByForumId
 *  Returns the index of a forum in the BoardsModel given a specific forumId.
 * @param forumId The forumId for which to start
 * @param startIdx The board's index, aka the point from which to start the search
 * @return QModelIndex
 */
QModelIndex BoardsModel::getIndexByForumId(const QString& forumId, QModelIndex startIdx)
{
    QModelIndex index;
    QStack<QModelIndex> stack;
    auto boardItem = itemFromIndex(startIdx);

    for (auto i=0; i< boardItem->rowCount(); i++)
    {
        stack.push(boardItem->child(i)->index());
    }

    while (!stack.isEmpty())
    {
        auto curItem = itemFromIndex(stack.pop());

        if (curItem->data().canConvert<ForumPtr>()
            && curItem->data().value<ForumPtr>()->getId() == forumId)
        {
            index = curItem->index();
            break;
        }

        if (curItem->hasChildren())
        {
            for (auto i=0; i< curItem->rowCount(); i++)
            {
                stack.push(curItem->child(i)->index());
            }
        }
    }

    return index;
}


QStandardItem* BoardsModel::getForumItem(ForumPtr f)
{
    QStandardItem* ret = nullptr;

    assert(f->getBoard().lock());
    auto board = f->getBoard().lock();

    if (board)
    {
        QStack<QStandardItem*> stack;
        auto boardItem = getBoardItem(board);

        for (auto i = 0; i < boardItem->rowCount(); i++)
        {
            stack.push_back(boardItem->child(i));
        }

        while (!stack.empty())
        {
            auto testItem = stack.pop();
            if (testItem->data().value<ForumPtr>() == f)
            {
                ret = testItem;
                break;
            }

            for (auto i = 0; i < testItem->rowCount(); i++)
            {
                stack.push_front(testItem->child(i));
            }
        }
    }

    return ret;
}

QString BoardsModel::getIndexKey(owl::BoardPtr b, BoardItemPtr bi)
{
    // boardDbId:forumId
    return QString("%1:%2").arg(b->getDBId()).arg(bi->getId());
}

} // namespace
