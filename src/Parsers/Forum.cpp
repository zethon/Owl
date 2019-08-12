#include <Utils/DateTimeParser.h>
#include "../Utils/Moment.h"
#include "../Utils/OwlUtils.h"
#include "Forum.h"

namespace owl
{

/**********************************************************/
/* BoardItem */
/**********************************************************/
void BoardItem::registerMeta()
{
	// do nothing
}


/**********************************************************/
/* Post */
/**********************************************************/
QString Post::getPrettyTimestamp(const DateTimeFormatOptions& options)
{
    QString retval;

    const QDateTime& datetime = getDateTime();
    if (!datetime.isNull() && datetime.isValid())
    {
        if (!options.useDefault)
        {
            owl::Moment moment(getDateTime());
            retval = moment.toString();
        }
        else
        {
            const auto now = QDateTime::currentDateTime();
            const int daysAgo = static_cast<int>(std::ceil(datetime.daysTo(now)));

            QString day { datetime.date().toString() };
            QString time { datetime.time().toString(options.timeFormat) };

            if (options.usePretty && daysAgo == 0)
            {
                day = tr("Today");
            }
            else if (options.usePretty && daysAgo == 1)
            {
                day = tr("Yesterday");
            }
            else
            {
                Qt::DateFormat format = Qt::TextDate;
                if (options.dateFormat == "short")
                {
                    format = Qt::SystemLocaleShortDate;
                }
                else if (options.dateFormat == "long")
                {
                    format = Qt::SystemLocaleLongDate;
                }

                day = datetime.date().toString(format);
            }

            retval = QString("%1 %2").arg(day).arg(time);
        }
    }
    else
    {
        retval = getDatelineString(); // assign the raw value from the post
    }

    return retval;
}


/**********************************************************/
/* Forum */
/**********************************************************/
Forum::Forum(const QString& id, const QString& name, const ForumType type)
	: BoardItem(id),
	  _name(name),
      _iDisplayOrder(0),
	  _bIsRoot(false),
	  _forumType(type),
      _modelItem(nullptr)
{
	// do nothing
};

Forum::Forum(const QString& id, const QString& name)
	: Forum(id, name, owl::Forum::FORUM)
{
	// do nothing
};
    
Forum::Forum(const QString& id)
    : Forum(id, QString(), owl::Forum::FORUM)
{
	// do nothing
}
    
Forum::~Forum()
{
	// do nothing
};

QString Forum::getForumTypeString() const
{
	QString retStr;

	switch (this->getForumType())
	{
		case Forum::FORUM:
			retStr = "FORUM";
		break;

		case Forum::CATEGORY:
			retStr = "CATEGORY";
		break;

		case Forum::LINK:
			retStr = "LINK";
		break;

		default:
            OWL_THROW_EXCEPTION(Exception("Unknown ForumType"));
	}
	 
	return retStr;
}

std::int32_t Forum::getLevel() const
{
    return _iForumLevel;
}

void BoardItem::setVar(const QString& key, const QString& value)
{
    _vars.erase(key);
	_vars.add(key, value);
}

QString BoardItem::getVar(const QString& key, bool bThrow /*= true*/)
{	
	return _vars.getText(key, bThrow);
}

bool Forum::isStructureEqual(Forum &other)
{
	auto bEqual = true;

	if (other.getId() == this->getId())
	{
		auto forumCount = getForums().size();

		if (forumCount == other.getForums().size())
		{
			// check children
			for (auto i = 0; i < getForums().size(); i++)
			{
				auto thisChild = _forums[i]->upCast<ForumPtr>(false);
				auto otherChild = other.getForums().at(i)->upCast<ForumPtr>(false);
				
				if (thisChild == nullptr || otherChild == nullptr 
					|| thisChild->Forum::operator!=(*otherChild)
					|| !thisChild->isStructureEqual(otherChild))
				{
					bEqual = false;
					break;
				}
			}
		}
		else
		{
			bEqual = false;
		}
	}
	else
	{
		bEqual = false;
	}

	return bEqual;
}

bool Forum::isStructureEqual(std::shared_ptr<Forum> other)
{
	return isStructureEqual(*other);
}

BoardItemPtr BoardItem::addChild(BoardItemPtr child, bool bThrow /*= true*/)
{
	QMutexLocker lock(&_childLock);

	if (child == nullptr)
	{
		if (bThrow)
		{
            OWL_THROW_EXCEPTION(Exception("Cannot add nullptr BoardItem as child"));
		}
		else
		{
			return BoardItemPtr();
		}
	}

	if (_children.contains(child))
	{
		if (bThrow)
		{
            OWL_THROW_EXCEPTION(Exception("BoardItem already exists as a child"));
		}
		else
		{
			return BoardItemPtr();
		}
	}

	_children.push_back(child);
    child->setParent(shared_from_this());

	return child;
}

void BoardItem::removeChild(BoardItemPtr child, bool bThrow)
{
	QMutexLocker lock(&_childLock);

	if (child == nullptr && bThrow)
	{
		if (bThrow)
		{
            OWL_THROW_EXCEPTION(Exception("Cannot remove nullptr BoardItem child"));
		}
		else
		{
			return;
		}
	}

	if (!_children.contains(child) && bThrow)
	{
		if (bThrow)
		{
            OWL_THROW_EXCEPTION(Exception("Cannot remove BoardItem that does not exists as a child"));
		}
		else
		{
			return;
		}
	}

	_children.removeOne(child);
	child->_parent.reset();
}

QString Thread::getFirstUnreadId() const
{
    if (_firstUnread)
    {
        return _firstUnread->getId();
    }

    return QString();
}

void Thread::setReplyCount(std::uint32_t var)
{
    _iReplyCount = var;
}

QString Thread::getPreviewText(uint maxLen /* =128 */ ) const
{
    return owl::previewText(_strPreviewText, maxLen);
}

} // namespace owl
