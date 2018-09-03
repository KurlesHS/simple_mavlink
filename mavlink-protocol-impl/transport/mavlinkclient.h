#pragma once

#include <QObject>

#include <entities/mavlinkcommand.h>

#include "imavlinktransport.h"

namespace rsvo {

namespace mavlinkprotocol {


class MavLinkClient : public QObject
{
    Q_OBJECT
public:
    explicit MavLinkClient(IMavlinkTransport *transport, QObject *parent = nullptr);

    bool sendCommand(const MavLinkCommandSharedPtr &command);
    bool sendCommand(const mavlink_message_t &message);
    IMavlinkTransport *transport() const;

signals:
    void mavLinkCommandReceived(const MavLinkCommandSharedPtr &command);

private: //slots
    void onReadyRead();

private:
    void handleReceivedData(const QByteArray &dataReceived);

private:
    IMavlinkTransport *mTransport;
    mavlink_status_t mStatus;

};

}

}
