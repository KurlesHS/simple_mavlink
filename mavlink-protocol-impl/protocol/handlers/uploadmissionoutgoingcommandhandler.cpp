#include "uploadmissionoutgoingcommandhandler.h"

#include <protocol/commands/uploadmissionoutgoingcommand.h>

#include <QPointer>
#include <QVector>
#include <QTimer>
#include <QDebug>

#include <transport/mavlinkclient.h>

#include <entities/mavlinkcommandcreator.h>

using namespace rsvo::mavlinkprotocol;

namespace {
constexpr int commandTimeout = 2000;
constexpr int maxRetries = 3;
}

namespace rsvo {

namespace mavlinkprotocol {

class UploadMissionOutgoingCommandHandlerPrivate : public QObject
{
public:
    UploadMissionOutgoingCommandHandlerPrivate(QObject *parent);

    bool processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient);
    bool incommingMavlinkCommand(const MavLinkCommandSharedPtr &command);

    bool handleMissionRequestCmd(const MavLinkCommandSharedPtr &command);
    bool handleMissionAckCmd(const MavLinkCommandSharedPtr &command);

private:
    void logMessage(const QString &message);
    void startUpload();
    void resendCmd();
    void finishCurrentCommand(const bool result);

private: // slots
    void onCommandTimeout();

private:
    UploadMissionOutgoingCommandHandler *q;
    QPointer<MavLinkClient> mClient;
    QTimer mCommandTimeoutTimer;
    QSharedPointer<UploadMissionOutgoingCommand> mCurrentCommand;
    MavLinkCommandCreator mCommandCreator;
    uint8_t mCurrentRetry;
    MavLinkCommandSharedPtr mCurrentMavlinkCmd;

};

}

}

UploadMissionOutgoingCommandHandlerPrivate::UploadMissionOutgoingCommandHandlerPrivate(QObject *parent) :
    QObject (parent),
    q(qobject_cast<UploadMissionOutgoingCommandHandler*>(parent))
{
    connect(&mCommandTimeoutTimer, &QTimer::timeout,
            this, &UploadMissionOutgoingCommandHandlerPrivate::onCommandTimeout);
    mCommandTimeoutTimer.setInterval(commandTimeout);
    mCommandTimeoutTimer.setSingleShot(true);
}

bool UploadMissionOutgoingCommandHandlerPrivate::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
    if (!mavlinkClient || !mavlinkClient->transport() || !mavlinkClient->transport()->isOpen()) {
        // нет транспорта
        logMessage(tr("Получили команду на аплоад маршрутного задания: проблемы с транспортом, отмена"));
        return false;
    }
    if (mCurrentCommand || !command) {
        // уже обрабатывает команду
        logMessage(tr("Получили команду на аплоад маршрутного задания во время заливки другого маршрутного задания, отмена"));
        return false;
    }
    mCurrentCommand = qSharedPointerObjectCast<UploadMissionOutgoingCommand>(command);
    if (!mCurrentCommand) {
        // команда не того типа
        logMessage(tr("Получили команду на аплоад маршрутного задания: внутренняя проблема (ошибка приведения типа), отмена"));
        return false;
    }
    if (mCurrentCommand->missionItems().isEmpty()) {
        logMessage(tr("Получили команду на аплоад маршрутного задания без маршрутных точек, отмена"));
        // пустой спиок маршрутного задания
        return false;
    }
    mCommandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    mClient = mavlinkClient;
    startUpload();
    return true;
}

bool UploadMissionOutgoingCommandHandlerPrivate::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    bool result = false;
    if (command->msg.msgid == MAVLINK_MSG_ID_MISSION_ACK) {
        result = handleMissionAckCmd(command);
    } else if (command->msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST) {
        result = handleMissionRequestCmd(command);
    }
    return result;
}

bool UploadMissionOutgoingCommandHandlerPrivate::handleMissionRequestCmd(const MavLinkCommandSharedPtr &command)
{
    // mavlink_mission_request_int_t resp;
    mavlink_mission_request_t resp;
    // mavlink_msg_mission_request_int_decode(&command->msg, &resp);
    mavlink_msg_mission_request_decode(&command->msg, &resp);
    if (resp.seq < mCurrentCommand->missionItems().size()) {
        auto item = mCurrentCommand->missionItems().at(resp.seq);
        if (item) {
            if (item->type() == IMissionItem::Type::Waypoint) {
                WaypointMissionItem *waypointItem = dynamic_cast<WaypointMissionItem*>(item.data());
                if (waypointItem) {
                    mCurrentRetry = maxRetries;
                    mCurrentMavlinkCmd = mCommandCreator.getNavWaypointMissionItemCommand(
                                static_cast<float>(waypointItem->latitude),
                                static_cast<float>(waypointItem->longitude),
                                static_cast<float>(waypointItem->altitude),
                                static_cast<float>(waypointItem->delay),
                                resp.seq);
                    mCommandTimeoutTimer.stop();
                    resendCmd();
                    logMessage(tr("Посылка %1 части маршрутного задания").arg(resp.seq + 1));
                    return true;
                } else {
                    logMessage(tr("Внутренняя ошибка: не удалость привести IMissionItem к WaypointMissionItem"));
                }
            }
        } else {
            logMessage(tr("Внутренняя ошибка: пустой IMissionItem списке айтемов в маршрутном задании"));
        }
    }
    return false;
}

bool UploadMissionOutgoingCommandHandlerPrivate::handleMissionAckCmd(const MavLinkCommandSharedPtr &command)
{
    mavlink_mission_ack_t resp;
    mavlink_msg_mission_ack_decode(&command->msg, &resp);
    logMessage(tr("mission uploader: окончание заливки маршрутного задания с результатом: %0").arg(resp.type == 0 ? tr("успех") : tr("ошибка")));
    finishCurrentCommand(true);
    return true;
}

void UploadMissionOutgoingCommandHandlerPrivate::logMessage(const QString &message)
{
    q->logMessage(message);
}

void UploadMissionOutgoingCommandHandlerPrivate::startUpload()
{
    mCurrentRetry = maxRetries;
    mCommandCreator.clearMissionSeq();
    int totalMissionItemCount = mCurrentCommand->missionItems().size();
    logMessage(tr("Подготовка к заливке %0 частей маршрутного задания").arg(totalMissionItemCount));
    mCurrentMavlinkCmd = mCommandCreator.getMissionCountCommand(static_cast<uint16_t>(totalMissionItemCount));
    resendCmd();
}

void UploadMissionOutgoingCommandHandlerPrivate::resendCmd()
{
    if (mCurrentRetry == 0 || !mClient) {
        finishCurrentCommand(false);
    } else {
        --mCurrentRetry;
        mClient->sendCommand(mCurrentMavlinkCmd);
        mCommandTimeoutTimer.start();
    }
}

void UploadMissionOutgoingCommandHandlerPrivate::finishCurrentCommand(const bool result)
{
    auto cmd = mCurrentCommand;
    mCurrentMavlinkCmd.clear();
    mCommandTimeoutTimer.stop();
    mCurrentCommand.clear();
    mClient.clear();
    if (cmd) {
        cmd->makeFinished(result);
    }
}

void UploadMissionOutgoingCommandHandlerPrivate::onCommandTimeout()
{
    mCommandTimeoutTimer.stop();
    resendCmd();
}

UploadMissionOutgoingCommandHandler::UploadMissionOutgoingCommandHandler(QObject *parent) :
    IOutgoingCommandHandler (parent),
    d(new UploadMissionOutgoingCommandHandlerPrivate(this))
{

}

IOutgoingCommand::Command rsvo::mavlinkprotocol::UploadMissionOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::UploadMission;
}

bool rsvo::mavlinkprotocol::UploadMissionOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
    return d->processCommand(command, systemId, componentId, targetSystemId, targetComponentId, mavlinkClient);
}

bool rsvo::mavlinkprotocol::UploadMissionOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    return d->incommingMavlinkCommand(command);
}
