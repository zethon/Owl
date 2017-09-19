// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <memory>
#include <QList>
#include <QObject>
#include <log4qt/logger.h>
#include <Utils/StringMap.h>

namespace owl
{

class StringMap;
class ParserBase;
using ParserBasePtr = std::shared_ptr<ParserBase>;

class Forum;
using ForumWeakPtr = std::weak_ptr<Forum>;

class BoardObject;
using BoardObjectPtr = std::shared_ptr<owl::BoardObject>;
using BoardObjectWeakPtr = std::weak_ptr<owl::BoardObject>;
using BoardObjectList = QList<BoardObjectPtr>;

class BoardObject : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

    Q_PROPERTY(QString name MEMBER _name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString url MEMBER _url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString serviceUrl MEMBER _serviceUrl WRITE setServiceUrl NOTIFY serviceUrlChanged)
    Q_PROPERTY(QString protocolName MEMBER _protocol WRITE setProtocolName NOTIFY protocolNameChanged)

    Q_PROPERTY(QString username MEMBER _username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password MEMBER _password WRITE setPassword NOTIFY passwordChanged)

    Q_PROPERTY(QByteArray iconBuffer MEMBER _iconBuffer WRITE setIconBuffer NOTIFY iconBufferChanged)
    Q_PROPERTY(bool enabled MEMBER _enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool autoLogin MEMBER _autoLogin WRITE setAutoLogin NOTIFY autoLoginChanged)
    Q_PROPERTY(Status status MEMBER _status WRITE setStatus NOTIFY statusChanged)

public:
    enum class Status
    {
        OFFLINE,
        ONLINE
    };
    Q_ENUM(Status)

    BoardObject(uint dbId)
        : _id(dbId)
    {
    }

    BoardObject(const QString& url);

    void setName(const QString& var) { _name = var; }
    void setUrl(const QString& var) { _url = var; }
    void setServiceUrl(const QString& var) { _serviceUrl = var; }
    void setProtocolName(const QString& var) { _protocol = var; }
    void setUsername(const QString& var) { _username = var; }
    void setPassword(const QString& var) { _password = var; }

    void setIconBuffer(const QByteArray& var) { _iconBuffer = var; }
    void setEnabled(bool var) { _enabled = var; }
    void setAutoLogin(bool var) { _autoLogin = var; }
    void setStatus(Status var) { _status = var; }

    StringMap& getOptions();
    const StringMap &getOptions() const;

    virtual void serialize(QVariantMap& data)
    {
        data["id"] = _id;
        data["name"] = _name;
        data["url"] = _url;
        data["serviceurl"] = _serviceUrl;
        data["protocol"] = _protocol;
    }

    virtual void deserialize(const QVariantMap& data)
    {
        _id = data["id"].toUInt();
        _name = data["name"].toString();
        _url = data["url"].toString();
        _serviceUrl = data["serviceurl"].toString();
        _protocol = data["protocol"].toString();
    }

Q_SIGNALS:
    void nameChanged();
    void urlChanged();
    void serviceUrlChanged();
    void protocolNameChanged();
    void iconBufferChanged();
    void enabledChanged();
    void autoLoginChanged();
    void statusChanged();
    void usernameChanged();
    void passwordChanged();

private:
    uint            _id = 0;
    QString         _name;
    QString         _url;
    QString         _serviceUrl;
    QString         _protocol;
    QString         _username;
    QString         _password;

    QByteArray      _iconBuffer;

    bool            _enabled = true;
    bool            _autoLogin = true;

    Status          _status;
    StringMap       _options;
    ParserBasePtr   _parser;
    ForumWeakPtr    _root;
};

} // namespace
