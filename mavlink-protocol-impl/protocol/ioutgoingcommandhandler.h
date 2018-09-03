#pragma once

#include <QObject>

#include "ioutgoingcommand.h"
#include <entities/mavlinkcommand.h>

namespace rsvo {

namespace mavlinkprotocol {

class MavLinkClient;

class IOutgoingCommandHandler : public QObject
{
    Q_OBJECT
public:
    explicit IOutgoingCommandHandler(QObject *parent = nullptr) : QObject(parent) {}

    virtual IOutgoingCommand::Command commandType() const = 0;
    virtual bool processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient) = 0;
    virtual bool incommingMavlinkCommand(const MavLinkCommandSharedPtr &command) = 0;

signals:
    void logMessage(const QString &message);

public slots:
};

}

}
