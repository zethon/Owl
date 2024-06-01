#include <QFile>

#include <fmt/format.h>

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa5.h"

#include "Data/BoardManager.h"
#include "ConnectionListModel.h"

namespace owl
{

ConnectionPtr ConnectionFromJson(const QJsonObject& object)
{
    ConnectionPtr retval;

    const auto displayOrder = object["displayOrder"].toInt();
    const auto uuidStr = object["uuid"].toString().toStdString();
    const auto typeStr = object["type"].toString().toLower();

    if (typeStr == "legacy_board")
    {
        auto board = BOARDMANAGER->boardByUUID(uuidStr);
        retval = std::make_shared<LegacyBoardConnection>(displayOrder, board);
    }
    else if (typeStr == "browser")
    {
        retval = std::make_shared<BrowserConnection>(uuidStr, displayOrder);
    }
    else if (typeStr == "reddit")
    {
        retval = std::make_shared<RedditConnection>(uuidStr, displayOrder);
    }
    else
    {
        const auto msg = fmt::format("Unknown connection type: {}", object["type"].toString().toStdString());
        OWL_THROW_EXCEPTION(owl::Exception(msg));
    }

    return retval;
}

LegacyBoardConnection::LegacyBoardConnection(std::uint16_t displayOrder, BoardPtr board)
    : Connection{board->uuid(), displayOrder}, _board{board}
{
    constexpr auto ICON_WIDTH = 128;
    constexpr auto ICON_HEIGHT = 128;
    QByteArray buffer(board->getFavIcon().toLatin1());
    QImage image = QImage::fromData(QByteArray::fromBase64(buffer));
    image = resizeImage(image, QSize(ICON_WIDTH, ICON_HEIGHT));
    _roleData[Qt::DecorationRole] = QIcon{ QPixmap::fromImage(image) };
    _roleData[owl::ConnectionRoles::DATA] = QVariant::fromValue(board);
}

BrowserConnection::BrowserConnection(const std::string& uuid, std::uint16_t displayOrder)
    : Connection{uuid, displayOrder}
{
    const auto icon { QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_globe)) };
    _roleData[Qt::DecorationRole] = icon;
}

RedditConnection::RedditConnection(const std::string& uuid, std::uint16_t displayOrder)
    : Connection{uuid, displayOrder}
{
    const auto icon { QIcon(ZFontIcon::icon(Fa5brands::FAMILY, Fa5brands::fa_reddit_alien)) };
    _roleData[Qt::DecorationRole] = icon;
}

ChatButtonConnection::ChatButtonConnection()
    : StaticButtonConnection{"ChatConnectionUUID", 0}
{
    const auto icon { QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_users)) };
    _roleData[Qt::DecorationRole] = icon;
}

NewConnectionButton::NewConnectionButton()
    : StaticButtonConnection{"NewConnectionUUID", 0}
{
    const auto icon { QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_plus_circle)) };
    _roleData[Qt::DecorationRole] = icon;
}

ConnectionListModel::ConnectionListModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

bool ConnectionListModel::load(const QString& filename)
{
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    _jsonData = std::make_unique<QJsonDocument>(QJsonDocument::fromJson(input.readAll()));
    const auto json = _jsonData->object();

    if (const QJsonValue v = json["connections"]; v.isArray())
    {
        const QJsonArray connections = v.toArray();
        _connections.clear();
        for (const QJsonValue& connection : connections)
        {
            _connections.push_back(ConnectionFromJson(connection.toObject()));
        }

        std::sort(_connections.begin(), _connections.end(),
            [](ConnectionPtr a, ConnectionPtr b) { return a->displayOrder() < b->displayOrder(); });

        this->createStaticButtons();
    }

    return true;
}

int ConnectionListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) return 0;
    return static_cast<int>(_connections.size());
}

QModelIndex ConnectionListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex{};

    std::size_t trow = static_cast<std::size_t>(row);
    if (trow < _connections.size())
    {
        return createIndex(row, column, _connections[row].get());
    }

    return createIndex(row, column);
}

QVariant ConnectionListModel::data(const QModelIndex & index, int role) const
{
    const auto row = static_cast<std::size_t>(index.row());
    if (!index.isValid()) return {};
    if (row > _connections.size()) return {};

    if (role == owl::ConnectionRoles::TYPE)
    {
        return QVariant::fromValue(_connections[row]->type());
    }

    return _connections[row]->data(role);
}

void ConnectionListModel::createStaticButtons()
{
    _connections.insert(_connections.begin(),
        std::make_shared<ChatButtonConnection>());

    _connections.push_back(
        std::make_shared<NewConnectionButton>());
}


//********************************
//* BoardIconModel ( TO BE DELETED!!!! )
//********************************

BoardIconModel::BoardIconModel(QObject *parent)
    : QAbstractListModel(parent),
    _boardManager(owl::BoardManager::instance())
{
    owl::BoardManager* manager = owl::BoardManager::instance().get();

    QObject::connect(manager, &BoardManager::onBeginAddBoard, this,
                     [this](int first) { beginInsertRows(QModelIndex{}, first, first); });

    QObject::connect(manager, &BoardManager::onEndAddBoard, this,
                     [this]() { endInsertRows(); });

    QObject::connect(manager, &BoardManager::onBeginRemoveBoard, this,
                     [this](int first) { beginRemoveRows(QModelIndex{}, first, first); });

    QObject::connect(manager, &BoardManager::onEndRemoveBoard, this,
                     [this]() { endRemoveRows(); });
}

QModelIndex BoardIconModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex{};

    Q_ASSERT(!parent.isValid());
    Q_ASSERT(column == 0);

    std::size_t trow = static_cast<std::size_t>(row);
    if (trow < _boardManager->getBoardCount())
    {
        return createIndex(row, column, _boardManager->boardByIndex(trow).get());
    }

    return createIndex(row, column);
}

int BoardIconModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0) return 0;
    return static_cast<int>(_boardManager->getBoardCount() + 2);
}

//** CODE TO BE DELETED BELOW THIS LINE **/

QImage resizeImage(const QImage& original, const QSize& size)
{
    QImage finalImage { original };
    qreal iXScale = static_cast<qreal>(size.width()) / static_cast<qreal>(finalImage.width());
    qreal iYScale = static_cast<qreal>(size.height()) / static_cast<qreal>(finalImage.height());
    if (iXScale > 1 || iXScale < 1 || iYScale > 1 || iYScale < 1)
    {
        QTransform transform;
        transform.scale(iXScale, iYScale);
        finalImage = finalImage.transformed(transform, Qt::SmoothTransformation);
    }

    return finalImage;
}

QVariant BoardIconModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return QVariant{};

    if (const int boardCount = static_cast<int>(_boardManager->getBoardCount());
        index.row() < boardCount)
    {
        switch (role)
        {
        case Qt::DecorationRole:
        {
            owl::Board* board = static_cast<owl::Board*>(index.internalPointer());
            Q_ASSERT(board);

            QByteArray buffer(board->getFavIcon().toLatin1());

            QImage image = QImage::fromData(QByteArray::fromBase64(buffer));
            image = resizeImage(image, QSize(ICONSCALEWIDTH, ICONSCALEHEIGHT));
            return QIcon { QPixmap::fromImage(image) };
        }

        case ICONTYPE_ROLE:
            return QVariant::fromValue(IconType::BOARDICON);

        case BOARDPTR_ROLE:
        {
            std::size_t trow = static_cast<std::size_t>(index.row());
            std::weak_ptr<owl::Board> retval { _boardManager->boardByIndex(trow) };
            return QVariant::fromValue(retval);
        }
        }
    }
    else if (index.row() == boardCount)
    {
        switch (role)
        {
        case Qt::DecorationRole:
            return QVariant { QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_plus_circle)) };

        case ICONTYPE_ROLE:
            return QVariant::fromValue(IconType::ADDICON);
        }
    }
    else
    {
        switch (role)
        {
        case Qt::ToolTipRole:
            qDebug() << "wewewew";
            break;

        case Qt::DecorationRole:
            return QVariant { QIcon(ZFontIcon::icon(Fa5::FAMILY, Fa5::fa_globe)) };

        case ICONTYPE_ROLE:
            return QVariant::fromValue(IconType::WEBICON);
        }
    }

    return QVariant{};
}


} // namespace owl
