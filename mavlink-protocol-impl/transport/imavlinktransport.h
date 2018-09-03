#pragma once

#include <QObject>

namespace rsvo {

namespace mavlinkprotocol {

class IMavlinkTransport : public QObject
{
    Q_OBJECT
public:
    enum class ConnectionStatus {
        Disconnected,
        Connecting,
        ConnectionError,
        Connected
    };
public:
    explicit IMavlinkTransport(QObject *parent = nullptr);
    virtual void open() = 0;
    virtual void close() = 0;
    virtual void write(const QByteArray &data) = 0;
    virtual QByteArray read(const qint64 maxSize) = 0;
    virtual QByteArray readAll() = 0;
    virtual bool isOpen() const = 0;

signals:
    void logMessage(const QString &message);
    void readyRead();
    void connectionStatusChanged(const IMavlinkTransport::ConnectionStatus &status);
};

}

}
Q_DECLARE_METATYPE(rsvo::mavlinkprotocol::IMavlinkTransport::ConnectionStatus)


