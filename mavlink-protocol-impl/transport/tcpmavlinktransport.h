#pragma once

#include "imavlinktransport.h"

#include <QTcpSocket>

namespace rsvo {

namespace mavlinkprotocol {

class TcpMavlinkTransport : public IMavlinkTransport
{
    Q_OBJECT
public:
    TcpMavlinkTransport(const QString &address, const quint16 port, QObject *parent = nullptr);
    ~TcpMavlinkTransport() override;

    // IMavlinkTransport interface
public:
    virtual void open() override;
    virtual void close() override;
    virtual void write(const QByteArray &data) override;
    virtual QByteArray read(const qint64 maxSize) override;
    virtual QByteArray readAll() override;
    virtual bool isOpen() const override;

private: //slots
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState state);

private:
    void handleUnconnectedState();
    void handleConnectedState();
    void handleConnectingState();

private:
    QTcpSocket *mSocket;
    const QString mAddress;
    const quint16 mPort;
    bool mIsConnected;
    int mTrying;
};

}

}
