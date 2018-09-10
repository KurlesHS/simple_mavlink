#include "startmissionoutgoingcommandhandler.h"

#include <transport/mavlinkclient.h>
#include <entities/mavlinkcommandcreator.h>
#include <protocol/commands/startmissionoutgoingcommand.h>

#include <QTimer>
#include <QDebug>

using namespace rsvo::mavlinkprotocol;

namespace {
constexpr int commandTimeout = 2000;
constexpr int maxRetries = 3;
}

StartMissionOutgoingCommandHandler::StartMissionOutgoingCommandHandler(QObject *parent) :
    IOutgoingCommandHandler(parent)
{
    connect(&mCommandTimeoutTimer, &QTimer::timeout,
            this, &StartMissionOutgoingCommandHandler::onCommandTimeout);
    mCommandTimeoutTimer.setInterval(commandTimeout);
    mCommandTimeoutTimer.setSingleShot(true);
}


IOutgoingCommand::Command rsvo::mavlinkprotocol::StartMissionOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::StartMission;
}

bool rsvo::mavlinkprotocol::StartMissionOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
    if (mCurrentCmd) {
        if (command) {
            QTimer::singleShot(10, command.data(), [command]() {
               command->makeFinished(false);
            });
        }
        return false;
    }
    if (!mavlinkClient || !mavlinkClient->transport() || !mavlinkClient->transport()->isOpen()) {
        // нет транспорта
        logMessage(tr("Получили команду запуска маршрутного задания: проблемы с транспортом, отмена"));
        return false;
    }

    auto startMissionCommand = qSharedPointerObjectCast<StartMissionOutgoingCommand>(command);
    if (!startMissionCommand) {
        logMessage(tr("Получили команду запуска маршрутного задания: внутренняя проблема (ошибка приведения типа), отмена"));
        return false;
    }
    mClient = mavlinkClient;
    MavLinkCommandCreator commandCreator;
    commandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    auto cmd = commandCreator.getMissionStartCommand(startMissionCommand->from(), startMissionCommand->to());
    mavlinkClient->sendCommand(cmd);
    mCurrentRetry = maxRetries;
    mCurrentCmd = command;
    mCommandTimeoutTimer.start();

    logMessage(tr("Посылаем команду запуска маршрутного задания (item от %0 до %1)")
               .arg(startMissionCommand->from())
               .arg(startMissionCommand->to()));

    return true;
}

bool rsvo::mavlinkprotocol::StartMissionOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    if (mCurrentCmd && command->msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
        mavlink_command_ack_t cmd;
        mavlink_msg_command_ack_decode(&command->msg, &cmd);
        if (cmd.command == MAV_CMD_MISSION_START) {
            mCommandTimeoutTimer.stop();
            qDebug() << Q_FUNC_INFO << cmd.result;
            if (cmd.result == MAV_RESULT_ACCEPTED) {
                mCurrentCmd->makeFinished(true);
            } else {
                mCurrentCmd->makeFinished(false);
            }
            mCurrentCmd.clear();
            return true;
        }
    }
    return false;
}

void StartMissionOutgoingCommandHandler::onCommandTimeout()
{
    if (mCurrentCmd && --mCurrentRetry > 0 && mClient) {
        mCommandTimeoutTimer.start();
    } else {
        mClient.clear();
        if (mCurrentCmd) {
            mCurrentCmd->makeFinished(false);
            mCurrentCmd.clear();
        }
    }
}
