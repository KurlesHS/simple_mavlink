#pragma once

#include "../ioutgoingcommandhandler.h"

namespace rsvo {

namespace mavlinkprotocol {

class MountControlOutgoingCommandHandler : public IOutgoingCommandHandler
{
    Q_OBJECT
public:
    explicit MountControlOutgoingCommandHandler(QObject *parent = nullptr);

    // IOutgoingCommandHandler interface
public:
    virtual IOutgoingCommand::Command commandType() const override;
    virtual bool processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient) override;
    virtual bool incommingMavlinkCommand(const MavLinkCommandSharedPtr &command) override;
};

}

}
