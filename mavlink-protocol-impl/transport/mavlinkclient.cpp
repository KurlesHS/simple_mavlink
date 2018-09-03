#include "mavlinkclient.h"

#include <mavlink/common/mavlink.h>

#include <QDebug>

using namespace rsvo::mavlinkprotocol;

MavLinkClient::MavLinkClient(IMavlinkTransport *transport, QObject *parent) :
    QObject(parent),
    mTransport(transport)
{
    connect(transport, &IMavlinkTransport::readyRead,
            this, &MavLinkClient::onReadyRead);
}

bool MavLinkClient::sendCommand(const MavLinkCommandSharedPtr &command)
{
    return sendCommand(command->msg);
}

bool MavLinkClient::sendCommand(const mavlink_message_t &message)
{
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &message);

    if (len > 0) {
        qDebug() << "send msg with id" << message.msgid;
        mTransport->write(QByteArray(reinterpret_cast<char*>(buffer), len));
    } else {
        qDebug() << "fail to send msg with id %0" << message.msgid;
    }
    return len > 0;
}

IMavlinkTransport *MavLinkClient::transport() const
{
    return mTransport;
}

void MavLinkClient::handleReceivedData(const QByteArray &dataReceived)
{
    uint8_t channel = 0;
    mavlink_message_t message;    
    for (const char byte : dataReceived) {
        if (!mavlink_parse_char(channel, static_cast<uint8_t>(byte),
                                &message, &mStatus)) {
            continue;
        }
        auto cmd = MavLinkCommandSharedPtr::create();
        memcpy(&cmd->msg, &message, sizeof (message));
        emit mavLinkCommandReceived(cmd);
    }

}

void MavLinkClient::onReadyRead()
{
    if (mTransport) {
        handleReceivedData(mTransport->readAll());
    }
}
