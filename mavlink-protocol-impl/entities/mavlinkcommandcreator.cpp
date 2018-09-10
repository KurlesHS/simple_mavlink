#include "mavlinkcommandcreator.h"

#include <mavlink/common/mavlink_msg_mission_item.h>
#include <mavlink/common/mavlink_msg_mission_count.h>

#include <QVector>

using namespace rsvo::mavlinkprotocol;

namespace {
//от 1000 до 2000 - градусы от 0 до 360. 1000 - 0; 2000 - 360.
quint16 gradToInt(float grad) {
    constexpr float oneGradus = 1001.f / 360.f;
    float_t x = grad / 360.f;
    if (x > 1) {
        grad /= (static_cast<int>(x) + 1);
    }

    uint16_t result = static_cast<uint16_t>(oneGradus * grad);
    if (result > 1000) {
        result = 1000;
    }
    return result + 1000;
}
}

MavLinkCommandCreator::MavLinkCommandCreator() :
    mSystemId(0),
    mComponentId(0),
    mTargetSystemId(0),
    mTargetComponentId(0)
{

}

MavLinkCommandSharedPtr MavLinkCommandCreator::getRcChanneslOverrideCommand(const float pitch, const float course)
{
    auto result = MavLinkCommandSharedPtr::create();
    // 7 канал по тангажу,  8 по курсу
    mavlink_msg_rc_channels_override_pack(mSystemId, mComponentId, &result->msg, mTargetSystemId, mTargetComponentId, 0, 0, 0, 0, 0, 0, gradToInt(pitch), gradToInt(course));
#if 0
    emit logMessage(tr("Подготовка сообщения rc_channels_override:"));
    emit logMessage(tr(" system_id: %0").arg(mSystemId));
    emit logMessage(tr(" component_id: %0").arg(mComponentId));
    emit logMessage(tr(" target_system_id: %0").arg(mTargetSystem));
    emit logMessage(tr(" target_component_id: %0").arg(mTargetComponent));
    emit logMessage(tr(" channel: %0").arg(mChannel));
    emit logMessage(tr(" chan_raw1 .. chan_raw6: 0x0000"));
    emit logMessage(tr(" chan_raw7: градусы: %0, записываемое: %1").arg(pitch).arg(gradToInt(pitch)));
    emit logMessage(tr(" chan_raw8: градусы: %0, записываемое: %1").arg(course).arg(gradToInt(course)));
#endif
    return result;
}

MavLinkCommandSharedPtr MavLinkCommandCreator::getDoMountControlCommand(const float pitch, const float course)
{
    auto result = MavLinkCommandSharedPtr::create();
    mavlink_msg_command_long_pack(mSystemId, mComponentId, &result->msg, mTargetSystemId, mTargetComponentId,
                                  MAV_CMD_DO_MOUNT_CONTROL, 1, pitch, 0, course, 0, 0, 0, MAV_MOUNT_MODE_MAVLINK_TARGETING);


    return result;
}

void MavLinkCommandCreator::setCommonParams(const uint8_t system, const uint8_t componet, const uint8_t targetSystem, const uint8_t targetComponent)
{
    mSystemId = system;
    mComponentId = componet;
    mTargetSystemId = targetSystem;
    mTargetComponentId = targetComponent;
}

void MavLinkCommandCreator::clearMissionSeq()
{

}

QVector<MavLinkCommandSharedPtr> MavLinkCommandCreator::getMountConfigureMissionItemCommands(const uint16_t seq)
{
    float latitude = 0;
    float longitude = 0;
    float altitude = 0;

    auto item1 = MavLinkCommandSharedPtr::create();
    mavlink_msg_mission_item_int_pack(
                mSystemId, mComponentId,
                &item1->msg,
                mTargetSystemId, mTargetComponentId,
                seq, 2,
                MAV_CMD_DO_MOUNT_CONFIGURE,
                seq == 0 ? 1 : 0,
                true, 4, 1, 1, 1,
                static_cast<int32_t>(latitude * 10000000),
                static_cast<int32_t>(longitude * 10000000),
                altitude,
                MAV_MISSION_TYPE_MISSION);


    auto item2 = MavLinkCommandSharedPtr::create();
    mavlink_msg_mission_item_int_pack(
                mSystemId, mComponentId,
                &item2->msg,
                mTargetSystemId, mTargetComponentId,
                seq + 1, 2,
                MAV_CMD_DO_MOUNT_CONTROL,
                0,
                true, 4, 1, 1, 1,
                static_cast<int32_t>(latitude * 10000000),
                static_cast<int32_t>(longitude * 10000000),
                altitude,
                MAV_MISSION_TYPE_MISSION);
    return { item1, item2 };
}

MavLinkCommandSharedPtr MavLinkCommandCreator::getNavWaypointMissionItemCommand(const float latitude, const float longitude, const float altitude, const float delay, const uint16_t seq)
{
    auto result = MavLinkCommandSharedPtr::create();
#if 0
    mavlink_msg_mission_item_int_pack(

                mSystemId, mComponentId,
                &result->msg,
                mTargetSystemId, mTargetComponentId,
                seq, 3,
                MAV_CMD_NAV_WAYPOINT,
                seq == 0 ? 1 : 0,
                true, delay, 0, 0, 0,
                static_cast<int32_t>(latitude * 10000000),
                static_cast<int32_t>(longitude * 10000000),
                altitude,
                MAV_MISSION_TYPE_MISSION);
#endif
    mavlink_msg_mission_item_pack(

                mSystemId, mComponentId,
                &result->msg,
                mTargetSystemId, mTargetComponentId,
                seq, 3,
                MAV_CMD_NAV_WAYPOINT,
                seq == 0 ? 1 : 0,
                true, delay, 0, 0, 0,
                latitude,
                longitude,
                altitude,
                MAV_MISSION_TYPE_MISSION);
    return result;

}

MavLinkCommandSharedPtr MavLinkCommandCreator::getNavReturnToLaunchCommand(const bool isMissionItem, const uint16_t seq)
{
    auto result = MavLinkCommandSharedPtr::create();
    if (isMissionItem) {
        mavlink_msg_mission_item_int_pack(
                    mSystemId, mComponentId,
                    &result->msg,
                    mTargetSystemId, mTargetComponentId,
                    seq, 2,
                    MAV_CMD_NAV_RETURN_TO_LAUNCH,
                    seq == 0 ? 1 : 0,
                    true, 0, 0, 0, 0,
                    0, 0, 0,
                    MAV_MISSION_TYPE_MISSION);
    } else {
#if 0
        mavlink_msg_command_int_pack(
                    mSystemId, mComponentId,
                    &result->msg,
                    mTargetSystemId, mTargetComponentId,
                    2,
                    MAV_CMD_NAV_RETURN_TO_LAUNCH,
                    1,
                    true, 0, 0, 0, 0,
                    0, 0, 0);
#endif
        mavlink_msg_command_long_pack(mSystemId, mComponentId,
                                      &result->msg,
                                      mTargetSystemId, mTargetComponentId,
                                      MAV_CMD_NAV_RETURN_TO_LAUNCH,
                                      true, 0, 0, 0, 0,
                                      0, 0, 0);

    }
    return result;
}

MavLinkCommandSharedPtr MavLinkCommandCreator::getMissionStartCommand(int from, int to)
{
    auto result = MavLinkCommandSharedPtr::create();
    mavlink_msg_command_long_pack(mSystemId, mComponentId,
                                  &result->msg,
                                  mTargetSystemId, mTargetComponentId,
                                  MAV_CMD_MISSION_START,
                                  true, from, to, 0, 0,
                                  0, 0, 0);
    return result;
}

MavLinkCommandSharedPtr MavLinkCommandCreator::getMissionCountCommand(const quint16 count)
{
    auto result = MavLinkCommandSharedPtr::create();
    mavlink_msg_mission_count_pack(
                mSystemId, mComponentId,
                &result->msg,
                mTargetSystemId, mTargetComponentId,
                count, MAV_MISSION_TYPE_MISSION);
    return result;
}



MavLinkCommandSharedPtr MavLinkCommandCreator::getMissionSetCurrentCommand(int current)
{

    auto result = MavLinkCommandSharedPtr::create();
    mavlink_msg_mission_set_current_pack(
                mSystemId, mComponentId,
                &result->msg,
                mTargetSystemId, mTargetComponentId,
                static_cast<uint16_t>(current));
    return result;
}

MavLinkCommandSharedPtr MavLinkCommandCreator::getSetModeCommand(int current)
{
    auto result = MavLinkCommandSharedPtr::create();
    mavlink_msg_set_mode_pack(
                mSystemId, mComponentId,
                &result->msg,
                mTargetSystemId, 217,
                static_cast<uint16_t>(current));
    return result;
}

MavLinkCommandSharedPtr MavLinkCommandCreator::getRequestProtocolCommand()
{
    auto result = MavLinkCommandSharedPtr::create();
    mavlink_msg_command_int_pack( mSystemId, mComponentId,
                                  &result->msg,
                                  mTargetSystemId, mTargetComponentId,
                                  0, MAV_CMD_REQUEST_PROTOCOL_VERSION,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0);
    return result;
}

uint8_t MavLinkCommandCreator::systemId() const
{
    return mSystemId;
}

uint8_t MavLinkCommandCreator::componentId() const
{
    return mComponentId;
}

uint8_t MavLinkCommandCreator::targetSystemId() const
{
    return mTargetSystemId;
}

uint8_t MavLinkCommandCreator::targetComponentId() const
{
    return mTargetComponentId;
}
