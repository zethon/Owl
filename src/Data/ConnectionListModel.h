#pragma once
#include <set>
#include <QAbstractListModel>
#include <QJsonDocument>

#include "Data/BoardManager.h" // needed?

namespace owl
{

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class Connection
{

public:
    Connection(const QString& uuid, std::uint16_t displayOrder)
        : _displayOrder{displayOrder}
    {
        // nothing to do
    }

    virtual ~Connection() = default;

    virtual QVariant data(int role)
    {
        if (!_roleData.contains(role)) return {};
        return _roleData[role];
    }

    std::uint16_t displayOrder() const { return _displayOrder; }
    QString uuid() const { return _uuid; }

protected:
    std::map<int, QVariant> _roleData;

private:
    std::uint16_t   _displayOrder;
    QString         _uuid;

};

struct ConnectionCmp
{
    bool operator() (ConnectionPtr a, ConnectionPtr b) const
    {
        return a->displayOrder() < b->displayOrder();
    }
};

class LegacyBoardConnection : public Connection
{
public:
    LegacyBoardConnection(const QString& uuid,
                          std::uint16_t displayOrder,
                          BoardPtr board);

private:
    BoardPtr    _board;
};

class BrowserConnection: public Connection
{
public:
    BrowserConnection(const QString& uuid, std::uint16_t displayOrder);
};

class RedditConnection: public Connection
{
public:
    RedditConnection(const QString& uuid, std::uint16_t displayOrder);
};

class StaticButtonConnection: public Connection
{
public:
    StaticButtonConnection(std::uint16_t displayOrder)
        : Connection({}, displayOrder)
    {
        // nothing to do
    }

    virtual ~StaticButtonConnection() = default;

};

class ChatButtonConnection : public StaticButtonConnection
{
public:
    ChatButtonConnection(std::uint16_t displayOrder);
};

class NewConnectionButton : public StaticButtonConnection
{
public:
    NewConnectionButton(std::uint16_t displayOrder);
};

// build a vector of `Connection` objects
class ConnectionListFactory
{

public:


};

class ConnectionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ConnectionListModel(QObject *parent = nullptr);
    ~ConnectionListModel() = default;

    bool load(const QString& filename);

private:
    // Inherited via `QAbstractItemModel`
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    void createStaticButtons();

    std::unique_ptr<QJsonDocument>  _jsonData;
    std::vector<ConnectionPtr>      _connections;   // the underlying connection data
};

// ****************************************************//
// ****************************************************//
// ** CODE THAT WILL BE DELETED GOES BELOW THIS LINE **//
// ****************************************************//
// ****************************************************//

using SpdLogPtr = std::shared_ptr<spdlog::logger>;

enum class IconType
{
    ADDICON, BOARDICON, WEBICON
};

} // owl

Q_DECLARE_METATYPE(owl::IconType);

namespace owl
{

QImage resizeImage(const QImage& original, const QSize& size);

class BoardIconModel : public QAbstractListModel
{
    Q_OBJECT

public:
    static constexpr auto ICONTYPE_ROLE = Qt::UserRole+1;
    static constexpr auto BOARDPTR_ROLE = Qt::UserRole+2;
    static constexpr auto ICONSCALEWIDTH = 128;
    static constexpr auto ICONSCALEHEIGHT = 128;

    explicit BoardIconModel(QObject *parent = nullptr);
    ~BoardIconModel() = default;

private:
    // Inherited via `QAbstractItemModel`
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    BoardManagerPtr     _boardManager;
};


} // namespace owl
