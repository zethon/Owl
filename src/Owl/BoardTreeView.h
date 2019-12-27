#pragma once

#include "BoardsModel.h"

namespace owl
{

class BoardTreeView : public QTreeView
{
	Q_OBJECT

	BoardsModel*	_boardModel = nullptr;
	QLabel*			_noBoardsConfigured = nullptr;

public:
	BoardTreeView(QWidget* parent = 0);
	virtual ~BoardTreeView() = default;

	void selectOneBelow(QModelIndex index);
    void selectForumBelow(const QModelIndex& index);

	void markForumRead(ForumPtr f);
	void updateForum(BoardPtr b, ForumPtr f);
	void expandIt(QModelIndex myIndex);
	void setHasBoards(bool bHasBoards);

    virtual void resizeEvent(QResizeEvent* event) override;
	virtual void setModel(QAbstractItemModel * model) override;

    void reloadView();

protected Q_SLOTS:
	virtual void currentChanged(const QModelIndex & current, const QModelIndex & previous) override;

Q_SIGNALS:
	void linkActivated(const QString &);

private:
    QString createStyle();
    void loadStyleSettings();
};

} // owl
