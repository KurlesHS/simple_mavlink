#include "mavlinkcommandhandler.h"

#include <QPointer>
#include <QVector>
#include <QTimer>
#include <QDebug>
#include <QDateTime>

#include <protocol/handlers/mountcontroloutgoingcommandhandler.h>
#include <protocol/handlers/returntohomeoutgoingcommandhandler.h>
#include <protocol/handlers/startmissionoutgoingcommandhandler.h>
#include <protocol/handlers/uploadmissionoutgoingcommandhandler.h>

#include <protocol/uavevents/altitudeuavevent.h>
#include <protocol/uavevents/batterystateuavevent.h>
#include <protocol/uavevents/globalpositionuavevent.h>
#include <protocol/uavevents/landedstateuavevent.h>
#include <protocol/uavevents/missionitemreacheduavevent.h>

using namespace rsvo::mavlinkprotocol;

namespace {
struct UavInfo
{
    QString id;
    uint8_t systemId;
    uint8_t componentId;
};

using uavEventHandler = std::function<IUavEventSharedPtr(const MavLinkCommandSharedPtr &command)>;

IUavEventSharedPtr extStateHandler (const MavLinkCommandSharedPtr &command) {
    mavlink_extended_sys_state_t msg;
    mavlink_msg_extended_sys_state_decode(&command->msg, &msg);

    // https://mavlink.io/en/messages/common.html#MAV_LANDED_STATE    
    return IUavEventSharedPtr(new LandedStateUavEvent(static_cast<LandedStateUavEvent::State>(msg.landed_state)));
}

IUavEventSharedPtr altitudeHandler (const MavLinkCommandSharedPtr &command) {
    mavlink_altitude_t msg;
    mavlink_msg_altitude_decode(&command->msg, &msg);

    // https://mavlink.io/en/messages/common.html#ALTITUDE    
    auto event = new AltitudeUavEvent;
    event->altitudeAmsl = msg.altitude_amsl;
    event->altitudeLocal = msg.altitude_local;
    event->altitudeRelative = msg.altitude_relative;
    event->altitudeTerrain = msg.altitude_terrain;
    return IUavEventSharedPtr(event);
}

IUavEventSharedPtr batteryStateHandler (const MavLinkCommandSharedPtr &command) {
    mavlink_battery_status_t msg;
    mavlink_msg_battery_status_decode(&command->msg, &msg);    
    auto event = new BatteryStateUavEvent;
    event->currentConsumed = msg.current_consumed;
    event->energyConsumed = msg.energy_consumed;
    event->temperature = msg.temperature;
    //event->voltages = msg.voltages;

    memcpy(event->voltages, msg.voltages, std::min(sizeof (event->voltages), sizeof (msg.voltages)));
    event->currentBattery = msg.current_battery;
    event->id = msg.id;
    event->batteryFunction = msg.battery_function;
    event->typeChemistry = msg.type;
    event->batteryRemaining = msg.battery_remaining;
    return IUavEventSharedPtr(event);
}

// https://mavlink.io/en/messages/common.html#GLOBAL_POSITION_INT
IUavEventSharedPtr globalPositionIntHandler (const MavLinkCommandSharedPtr &command) {
    static QDateTime lastTime = QDateTime::currentDateTime();
    mavlink_global_position_int_t msg;

    mavlink_msg_global_position_int_decode(&command->msg, &msg);

    if (lastTime.secsTo(QDateTime::currentDateTime()) >= 1) {
        qDebug() << "global position received" << msg.lat / 10000000. << msg.lon /  10000000. << msg.alt / 1000. << msg.relative_alt / 1000.  << msg.lat << msg.lon;
        lastTime = QDateTime::currentDateTime();
    }
    auto event = new GlobalPositionUavEvent;
    event->lat = msg.lat / 10000000.f;
    event->lon = msg.lon /  10000000.f;
    event->alt = msg.alt / 1000.f;
    event->relative_alt = msg.relative_alt / 1000.f;
    return IUavEventSharedPtr(event);
}

// https://mavlink.io/en/messages/common.html#MISSION_ITEM_REACHED
IUavEventSharedPtr missionItemReachedHandler (const MavLinkCommandSharedPtr &command) {
    mavlink_mission_item_reached_t msg;
    mavlink_msg_mission_item_reached_decode(&command->msg, &msg);    
    auto event = new MissionItemReachedUavEvent;
    event->seq = msg.seq;
    return IUavEventSharedPtr(event);
}

}


namespace rsvo {

namespace mavlinkprotocol {

class MavlinkCommandHandlerPrivate : public QObject
{
public:
    MavlinkCommandHandlerPrivate(MavLinkClient *client, const uint8_t systemId, const uint8_t componentId, QObject *parent);

    MavLinkClient *client() const;
    uint8_t systemId() const;
    uint8_t componentId() const;

    bool addCommand(const QString &uavId, IOutgoingCommandSharedPtr command);

    void registerOutgoingCommandHandler(IOutgoingCommandHandler *handler);
    void addUav(const QString &uavId, const uint8_t systemId, const uint8_t componentId);

    IOutgoingCommandHandler *handler(const IOutgoingCommand::Command commandType) const;

private:
    void onPingTimeout();
    void onMavLinkCommandReceived(const MavLinkCommandSharedPtr &command);
    void onTransportConnectionStatusChanged(const IMavlinkTransport::ConnectionStatus &status);

private:
    MavlinkCommandHandler *q;
    MavLinkClient *mClient;
    const uint8_t mSystemId;
    const uint8_t mComponentId;
    QHash<QString, UavInfo> mRegisteredUavs;
    IMavlinkTransport::ConnectionStatus mConnectionStatus;
    QVector<QPointer<IOutgoingCommandHandler> > mOutgoingCommandHandlers;
    QTimer *mPingTimer;
    QVector<QPair<uint16_t, uavEventHandler>> mUavEventHandlers;
    UploadMissionOutgoingCommandHandler mUploadMissionOutgoingCommandHandler;
    MountControlOutgoingCommandHandler mMountControlCmdHander;
    ReturnToHomeOutgoingCommandHandler mReturnToHomeOutgoingCommandHandler;
    StartMissionOutgoingCommandHandler mStartMissionOutgoingCommandHandler;
    bool mCurrentLinkStatus;
    int mPingCommandsWithoutIncommingCommand;
};

}

}

MavlinkCommandHandlerPrivate::MavlinkCommandHandlerPrivate(MavLinkClient *client, const uint8_t systemId, const uint8_t componentId, QObject *parent) :
    QObject(parent),
    q(qobject_cast<MavlinkCommandHandler*>(parent)),
    mClient(client),
    mSystemId(systemId),
    mComponentId(componentId),
    mConnectionStatus(IMavlinkTransport::ConnectionStatus::Disconnected),
    mPingTimer(new QTimer(this)),
    mCurrentLinkStatus(false),
    mPingCommandsWithoutIncommingCommand(0)
{
    registerOutgoingCommandHandler(&mUploadMissionOutgoingCommandHandler);
    registerOutgoingCommandHandler(&mMountControlCmdHander);
    registerOutgoingCommandHandler(&mReturnToHomeOutgoingCommandHandler);
    registerOutgoingCommandHandler(&mStartMissionOutgoingCommandHandler);

    mUavEventHandlers.append({MAVLINK_MSG_ID_EXTENDED_SYS_STATE,  &extStateHandler});
    mUavEventHandlers.append({MAVLINK_MSG_ID_BATTERY_STATUS,  &batteryStateHandler});
    mUavEventHandlers.append({MAVLINK_MSG_ID_GLOBAL_POSITION_INT, &globalPositionIntHandler});
    mUavEventHandlers.append({MAVLINK_MSG_ID_MISSION_ITEM_REACHED , &missionItemReachedHandler});
    mUavEventHandlers.append({MAVLINK_MSG_ID_ALTITUDE , &altitudeHandler});
    if (client) {
        connect(client, &MavLinkClient::mavLinkCommandReceived,
                this, &MavlinkCommandHandlerPrivate::onMavLinkCommandReceived);
        auto transport = client->transport();
        if (transport) {
            connect(transport, &IMavlinkTransport::connectionStatusChanged,
                    this, &MavlinkCommandHandlerPrivate::onTransportConnectionStatusChanged);
            connect(transport, &IMavlinkTransport::logMessage,
                    q, &MavlinkCommandHandler::logMessage);

        }
    }
    connect(mPingTimer, &QTimer::timeout,
            this, &MavlinkCommandHandlerPrivate::onPingTimeout);
    mPingTimer->setInterval(1000);
}

MavlinkCommandHandler::MavlinkCommandHandler(MavLinkClient *client, const uint8_t systemId, const uint8_t componentId, QObject *parent) :
    QObject(parent),
    d(new MavlinkCommandHandlerPrivate(client, systemId, componentId, this))
{

}

void MavlinkCommandHandler::registerOutgoingCommandHandler(IOutgoingCommandHandler *handler)
{
    d->registerOutgoingCommandHandler(handler);
}

void MavlinkCommandHandler::addUav(const QString &uavId, const uint8_t systemId, const uint8_t componentId)
{
    d->addUav(uavId, systemId, componentId);
}

bool MavlinkCommandHandler::addCommand(const QString &uavId, IOutgoingCommandSharedPtr command)
{
    return d->addCommand(uavId, command);
}

MavLinkClient *MavlinkCommandHandlerPrivate::client() const
{
    return mClient;
}

uint8_t MavlinkCommandHandlerPrivate::systemId() const
{
    return mSystemId;
}

uint8_t MavlinkCommandHandlerPrivate::componentId() const
{
    return mComponentId;
}

bool MavlinkCommandHandlerPrivate::addCommand(const QString &uavId, IOutgoingCommandSharedPtr command)
{
    auto it = mRegisteredUavs.find(uavId);
    if (it == mRegisteredUavs.end()) {
        return false;
    }
    IOutgoingCommandHandler * handler = this->handler(command->commandType());
    if (!handler) {
        return false;
    }
    return handler->processCommand(command, mSystemId, mComponentId, it.value().systemId, it.value().componentId, mClient);
}

void MavlinkCommandHandlerPrivate::registerOutgoingCommandHandler(IOutgoingCommandHandler *handler)
{
    auto h = this->handler(handler->commandType());
    if (!h) {
        mOutgoingCommandHandlers.append(handler);
        connect(handler, &IOutgoingCommandHandler::logMessage,
                q, &MavlinkCommandHandler::logMessage);
    }
}

void MavlinkCommandHandlerPrivate::addUav(const QString &uavId, const uint8_t systemId, const uint8_t componentId)
{
    mRegisteredUavs[uavId] = { uavId, systemId, componentId };
}

IOutgoingCommandHandler *MavlinkCommandHandlerPrivate::handler(const IOutgoingCommand::Command commandType) const
{
    for (auto it = mOutgoingCommandHandlers.begin(); it != mOutgoingCommandHandlers.end(); ++it) {
        if (it->data() && it->data()->commandType() == commandType) {
            return *it;
        }
    }
    return nullptr;
}

void MavlinkCommandHandlerPrivate::onPingTimeout()
{
    if (mConnectionStatus == IMavlinkTransport::ConnectionStatus::Connected) {
        mavlink_message_t msg;
        mavlink_msg_heartbeat_pack(mSystemId, mComponentId,  &msg, MAV_TYPE_GCS, 0, 0, 0, MAV_STATE_STANDBY);
        mClient->sendCommand(msg);
    }
    if (mCurrentLinkStatus) {
        ++mPingCommandsWithoutIncommingCommand;
        if (mPingCommandsWithoutIncommingCommand > 6) {
            mCurrentLinkStatus = false;
            emit q->linkStatus(false);
        }
    }
}

void MavlinkCommandHandlerPrivate::onMavLinkCommandReceived(const MavLinkCommandSharedPtr &command)
{    
    // qDebug() << "inc cmd" << command->msg.msgid;
    mPingCommandsWithoutIncommingCommand = 0;
    if (!mCurrentLinkStatus) {
        mCurrentLinkStatus = true;
        emit q->linkStatus(true);
    }    
    for (const auto &handerInfo : mUavEventHandlers) {
        if (handerInfo.first == command->msg.msgid) {
            IUavEventSharedPtr resut = handerInfo.second(command);
            auto it = mRegisteredUavs.begin();
            if (resut && it != mRegisteredUavs.end()) {
                emit q->uavEvent(it.key(), resut);
                return;
            }
        }
    }

    for (auto handler : mOutgoingCommandHandlers) {
        if (handler->incommingMavlinkCommand(command)) {
            return;
        }
    }
}

void MavlinkCommandHandlerPrivate::onTransportConnectionStatusChanged(const IMavlinkTransport::ConnectionStatus &status)
{
    mConnectionStatus = status;
    if (status == IMavlinkTransport::ConnectionStatus::Connected) {
        mPingTimer->start();        
    } else {
        mPingTimer->stop();
        if (mCurrentLinkStatus) {
            mCurrentLinkStatus = false;
            emit q->linkStatus(false);
        }
    }
}
