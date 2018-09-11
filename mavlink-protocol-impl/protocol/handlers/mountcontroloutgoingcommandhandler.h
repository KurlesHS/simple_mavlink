#pragma once

#include "../ioutgoingcommandhandler.h"

#include <QPointer>
#include <QTimer>

#include <transport/mavlinkclient.h>

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

private: // slots
    void onCommandTimeout();

private:
    QTimer mCommandTimeoutTimer;
    uint8_t mCurrentRetry;
    IOutgoingCommandSharedPtr mCurrentCmd;
    QPointer<MavLinkClient> mClient;
    float mLastPitch;
    float mLastCourse;
    float mCurrentPitch;
    float mCurrentCourse;
};

}

}
