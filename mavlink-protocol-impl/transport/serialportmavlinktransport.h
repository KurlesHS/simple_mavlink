#pragma once

#include <transport/imavlinktransport.h>

#include <QSerialPort>

namespace rsvo {

namespace mavlinkprotocol {


class SerialPortMavlinkTransport : public IMavlinkTransport
{
    Q_OBJECT
public:
    SerialPortMavlinkTransport(const QString &serialPortName, const quint32 baudrate, QObject *parent = nullptr);
    ~SerialPortMavlinkTransport() override;

    // IMavlinkTransport interface
public:
    virtual void open() override;
    virtual void close() override;
    virtual void write(const QByteArray &data) override;
    virtual QByteArray read(const qint64 maxSize) override;
    virtual QByteArray readAll() override;
    virtual bool isOpen() const override;

private:
    void onErrorOccurred(QSerialPort::SerialPortError error);

    void reconnect();

private:
    QSerialPort *mSerialPort;
    const QString mSerialPortName;
    const quint32 mBaudrate;
};

}

}


