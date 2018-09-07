#pragma once

#include "../ioutgoingcommandhandler.h"

#include <QPointer>
#include <QTimer>

#include <protocol/commands/startmissionoutgoingcommand.h>

#include <transport/mavlinkclient.h>


namespace rsvo {

namespace mavlinkprotocol {

class StartMissionOutgoingCommandHandler : public IOutgoingCommandHandler
{
    Q_OBJECT
public:
    explicit StartMissionOutgoingCommandHandler(QObject *parent = nullptr);

    // IOutgoingCommandHandler interface
public:
    virtual IOutgoingCommand::Command commandType() const override;
    virtual bool processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient) override;
    virtual bool incommingMavlinkCommand(const MavLinkCommandSharedPtr &command) override;

private: // slots
    void onCommandTimeout();

private:
    QTimer mCommandTimeoutTimer;
    uint8_t mCurrentRetry;
    IOutgoingCommandSharedPtr mCurrentCmd;
    QPointer<MavLinkClient> mClient;

};

}

}
