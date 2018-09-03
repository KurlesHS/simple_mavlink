#pragma once

#include "mavlinkcommand.h"

namespace rsvo {

namespace mavlinkprotocol {

class MavLinkCommandCreator
{
public:
    MavLinkCommandCreator();

    void setCommonParams(
            const uint8_t system, const uint8_t componet,
            const uint8_t targetSystem, const uint8_t targetComponent);
    void clearMissionSeq();

    uint8_t systemId() const;
    uint8_t componentId() const;
    uint8_t targetSystemId() const;
    uint8_t targetComponentId() const;

    QVector<MavLinkCommandSharedPtr> getMountConfigureMissionItemCommands(const uint16_t seq);
    MavLinkCommandSharedPtr getNavWaypointMissionItemCommand(
            const float latitude,
            const float longitude,
            const float altitude,
            const float delay,
            const uint16_t seq);
    MavLinkCommandSharedPtr getNavReturnToLaunchCommand(const bool isMissionItem=true, const uint16_t seq=0);
    MavLinkCommandSharedPtr getMissionCountCommand(const quint16 count);
    MavLinkCommandSharedPtr getMissionStartCommand(int from, int to);
    MavLinkCommandSharedPtr getMissionSetCurrentCommand(int current);
    MavLinkCommandSharedPtr getSetModeCommand(int current);
    MavLinkCommandSharedPtr getRequestProtocolCommand();    
    MavLinkCommandSharedPtr getRcChanneslOverrideCommand(const float pitch, const float course);

private:
    uint8_t mSystemId;
    uint8_t mComponentId;
    uint8_t mTargetSystemId;
    uint8_t mTargetComponentId;
};

}

}
