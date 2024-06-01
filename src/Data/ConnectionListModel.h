#pragma once
#include <set>
#include <QAbstractListModel>
#include <QJsonDocument>

#include "Data/BoardManager.h" // needed?

namespace owl
{

namespace ConnectionRoles
{
    static constexpr auto UUID = Qt::UserRole;
    static constexpr auto TYPE = Qt::UserRole+1;
    static constexpr auto DATA = Qt::UserRole+2;
}

enum class ConnectionType
{
    LEGACY_BOARD, BROWSER, REDDIT, CHAT_BUTTON, NEW_CONNECTION_BUTTON
};

} // namespace owl

Q_DECLARE_METATYPE(owl::ConnectionType);

namespace owl
{

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class Connection
{

public:
    Connection(const std::string& uuid, std::uint16_t displayOrder)
        : _displayOrder{displayOrder}
    {
        _roleData[owl::ConnectionRoles::UUID] = QString::fromStdString(uuid);
    }

    virtual ~Connection() = default;

    virtual QVariant data(int role)
    {
        if (!_roleData.contains(role)) return {};
        if (role == owl::ConnectionRoles::TYPE)
        {
            return QVariant::fromValue(this->type());
        }
        return _roleData[role];
    }

    virtual ConnectionType type() const = 0;

    std::uint16_t displayOrder() const { return _displayOrder; }
    std::string uuid() const 
    { 
        const auto uuid = _roleData.at(owl::ConnectionRoles::UUID).toString();
        return uuid.toStdString();
    }

protected:
    std::map<int, QVariant> _roleData;

private:
    std::uint16_t   _displayOrder;
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
    LegacyBoardConnection(std::uint16_t displayOrder,
                          BoardPtr board);

    virtual ~LegacyBoardConnection() = default;

    ConnectionType type() const override { return ConnectionType::LEGACY_BOARD; }

private:
    BoardPtr    _board;
};

class BrowserConnection: public Connection
{
public:
    BrowserConnection(const std::string& uuid, std::uint16_t displayOrder);
    virtual ~BrowserConnection() = default;

    ConnectionType type() const override { return ConnectionType::BROWSER; }
};

class RedditConnection: public Connection
{
public:
    RedditConnection(const std::string& uuid, std::uint16_t displayOrder);
    virtual ~RedditConnection() = default;

    ConnectionType type() const override { return ConnectionType::REDDIT; }
};

class StaticButtonConnection: public Connection
{
public:
    StaticButtonConnection(const std::string& uuid, std::uint16_t displayOrder)
        : Connection(uuid, displayOrder)
    {
        // nothing to do
    }

    virtual ~StaticButtonConnection() = default;

};

class ChatButtonConnection : public StaticButtonConnection
{
public:
    ChatButtonConnection();
    ~ChatButtonConnection() = default;

    ConnectionType type() const override { return ConnectionType::CHAT_BUTTON; }
};

class NewConnectionButton : public StaticButtonConnection
{
public:
    NewConnectionButton();
    ~NewConnectionButton() = default;

    ConnectionType type() const override { return ConnectionType::NEW_CONNECTION_BUTTON; }
};

class ConnectionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ConnectionListModel(QObject *parent = nullptr);
    ~ConnectionListModel() = default;

    bool load(const QString& filename);
    const std::vector<ConnectionPtr>& connections() const { return _connections; }

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
    static constexpr auto ICONTYPE_ROLE = Qt::UserRole+991;
    static constexpr auto BOARDPTR_ROLE = Qt::UserRole+992;
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
