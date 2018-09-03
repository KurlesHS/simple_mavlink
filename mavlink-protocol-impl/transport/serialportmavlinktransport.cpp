#include "serialportmavlinktransport.h"

#include <QTimer>

using namespace rsvo::mavlinkprotocol;

SerialPortMavlinkTransport::SerialPortMavlinkTransport(const QString &serialPortName, const quint32 baudrate, QObject *parent) :
    IMavlinkTransport(parent),
    mSerialPort(nullptr),
    mSerialPortName(serialPortName),
    mBaudrate(baudrate)
{

}

SerialPortMavlinkTransport::~SerialPortMavlinkTransport()
{    
    close();
}

void SerialPortMavlinkTransport::open()
{
    if (!mSerialPort) {
        mSerialPort = new QSerialPort(this);
        mSerialPort->setPortName(mSerialPortName);
        connect(mSerialPort, &QSerialPort::readyRead,
                this, &SerialPortMavlinkTransport::readyRead);
#if 0
        connect(mSerialPort, &QSerialPort::errorOccurred,
                this, &SerialPortMavlinkTransport::onErrorOccurred);
#endif

#if 0
        if (!mSerialPort->setDataBits(QSerialPort::Data8)) {
            return false;
        }
        if (!mSerialPort->setFlowControl(QSerialPort::NoFlowControl)) {
            return false;
        }
        if (!mSerialPort->setStopBits(QSerialPort::OneStop)) {
            return false;
        }
        if (!mSerialPort->setParity(QSerialPort::NoParity)) {
            return false;
        }

#endif
        bool ok = false;
        emit connectionStatusChanged(ConnectionStatus::Connecting);
        if (mSerialPort->open(QIODevice::ReadWrite)) {
            if (mSerialPort->setBaudRate(static_cast<const qint32>(mBaudrate))) {
                if (mSerialPort->setDataBits(QSerialPort::Data8)) {
                    if (mSerialPort->setFlowControl(QSerialPort::NoFlowControl)) {
                        if (mSerialPort->setStopBits(QSerialPort::OneStop)) {
                            if (mSerialPort->setParity(QSerialPort::NoParity)) {
                                ok = true;
                            }
                        }
                    }
                }
            }
        }
        if (ok) {
            emit connectionStatusChanged(ConnectionStatus::Connected);
        } else {
            emit connectionStatusChanged(ConnectionStatus::Disconnected);
            mSerialPort->disconnect();
            if (mSerialPort->isOpen()) {
                mSerialPort->close();
            }
            mSerialPort->deleteLater();
            mSerialPort = nullptr;
            QTimer::singleShot(1000, this, &SerialPortMavlinkTransport::reconnect);
        }
    }
}

void SerialPortMavlinkTransport::close()
{
    if (mSerialPort) {
        mSerialPort->disconnect();
        if (mSerialPort->isOpen()) {
            mSerialPort->close();
        }
        mSerialPort = nullptr;
        mSerialPort->deleteLater();
        emit connectionStatusChanged(ConnectionStatus::Disconnected);
    }
}

void SerialPortMavlinkTransport::write(const QByteArray &data)
{
    if (mSerialPort && mSerialPort->isOpen()) {
        mSerialPort->write(data);
        mSerialPort->flush();
    }
}

void SerialPortMavlinkTransport::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    if (mSerialPort) {
        mSerialPort->disconnect();
        if (mSerialPort->isOpen()) {
            mSerialPort->close();
        }
        mSerialPort = nullptr;
        mSerialPort->deleteLater();
    }
    emit connectionStatusChanged(ConnectionStatus::Disconnected);
    QTimer::singleShot(1000, this, &SerialPortMavlinkTransport::reconnect);
}

void SerialPortMavlinkTransport::reconnect()
{
    close();
    open();
}


QByteArray SerialPortMavlinkTransport::read(const qint64 maxSize)
{
    if (mSerialPort) {
        return mSerialPort->read(maxSize);
    }
    return QByteArray();
}

QByteArray SerialPortMavlinkTransport::readAll()
{
    if (mSerialPort) {
        return mSerialPort->readAll();
    }
    return QByteArray();
}

bool SerialPortMavlinkTransport::isOpen() const
{
    return mSerialPort && mSerialPort->isOpen();
}
