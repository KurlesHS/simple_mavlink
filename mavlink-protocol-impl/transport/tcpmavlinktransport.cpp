#include "tcpmavlinktransport.h"

#include <QTimer>

using namespace rsvo::mavlinkprotocol;

namespace {
constexpr int loudConnectionCount = 3;
}

TcpMavlinkTransport::TcpMavlinkTransport(const QString &address, const quint16 port, QObject *parent) :
    IMavlinkTransport(parent),
    mSocket(nullptr),
    mAddress(address),
    mPort(port),
    mTrying(loudConnectionCount)
{

}

TcpMavlinkTransport::~TcpMavlinkTransport()
{
    close();
}

void TcpMavlinkTransport::onStateChanged(QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        handleUnconnectedState();
        break;
    case QAbstractSocket::ConnectingState:
        handleConnectingState();
        break;
    case QAbstractSocket::ConnectedState:
        handleConnectedState();
        break;
    default:
        break;
    }
}

void TcpMavlinkTransport::handleUnconnectedState()
{
    if (mIsConnected) {
        emit connectionStatusChanged(ConnectionStatus::Disconnected);
        emit logMessage(tr("Соединение с TCP сервером  %0:%1 потеряно").arg(mAddress).arg(mPort));
    } else {
        emit connectionStatusChanged(ConnectionStatus::ConnectionError);
        if (mTrying <= 0) {

        } else {
            if (mTrying-- > 0) {
                emit logMessage(tr("Попытка установить соединение с TCP сервером %0:%1 не увенчалась успехом").arg(mAddress).arg(mPort));
            }
            if (mTrying == 0) {
                emit logMessage(tr("Следующие попытки установить соединение с TCP сервером %0:%1 будут проходить без вывода сообщений в журнал").arg(mAddress).arg(mPort));
            }
        }
    }
    mIsConnected = false;
    mSocket->disconnect();
    mSocket->deleteLater();
    mSocket = nullptr;
    QTimer::singleShot(1000, this, &TcpMavlinkTransport::open);
}

void TcpMavlinkTransport::handleConnectedState()
{
    mIsConnected = true;
    mTrying = loudConnectionCount;
    emit logMessage(tr("Соединение с TCP сервером %0:%1 установлено").arg(mAddress).arg(mPort));
    emit connectionStatusChanged(ConnectionStatus::Connected);
}

void TcpMavlinkTransport::handleConnectingState()
{
    mIsConnected = false;
    if (mTrying) {
        emit logMessage(tr("Попытка установить соединение с TCP сервером %0:%1").arg(mAddress).arg(mPort));
    }
    emit connectionStatusChanged(ConnectionStatus::Connecting);
}

void TcpMavlinkTransport::open()
{
    if (!mSocket) {
        mSocket = new QTcpSocket(this);
        connect(mSocket, &QTcpSocket::readyRead,
                this, &TcpMavlinkTransport::readyRead);
        connect(mSocket, &QTcpSocket::stateChanged,
                this, &TcpMavlinkTransport::onStateChanged);
        mSocket->connectToHost(mAddress, mPort);
    }
}

void TcpMavlinkTransport::close()
{
    if (mSocket) {
        mSocket->disconnect();
        mSocket->disconnectFromHost();
        mSocket->deleteLater();
        mSocket = nullptr;
        emit connectionStatusChanged(ConnectionStatus::Disconnected);
    }
}

void TcpMavlinkTransport::write(const QByteArray &data)
{
    if (mSocket && mSocket->state() == QAbstractSocket::ConnectedState) {
        mSocket->write(data);
    }
}

QByteArray TcpMavlinkTransport::read(const qint64 maxSize)
{
    if (mSocket) {
        return mSocket->read(maxSize);
    }
    return QByteArray();
}

QByteArray TcpMavlinkTransport::readAll()
{
    if (mSocket) {
        return mSocket->readAll();
    }
    return QByteArray();
}

bool TcpMavlinkTransport::isOpen() const
{
    return mSocket && mSocket->isOpen();
}
