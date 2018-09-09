#include "returntohomeoutgoingcommandhandler.h"

#include <transport/mavlinkclient.h>
#include <entities/mavlinkcommandcreator.h>
#include <protocol/commands/returntohomeoutgoingcommand.h>

#include <QTimer>
#include <QDebug>


using namespace rsvo::mavlinkprotocol;

namespace {
constexpr int commandTimeout = 2000;
constexpr int maxRetries = 3;
}


ReturnToHomeOutgoingCommandHandler::ReturnToHomeOutgoingCommandHandler(QObject *parent) : IOutgoingCommandHandler(parent)
{
    connect(&mCommandTimeoutTimer, &QTimer::timeout,
            this, &ReturnToHomeOutgoingCommandHandler::onCommandTimeout);
    mCommandTimeoutTimer.setInterval(commandTimeout);
    mCommandTimeoutTimer.setSingleShot(true);
}


IOutgoingCommand::Command ReturnToHomeOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::ReturnToHome;

}

bool ReturnToHomeOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
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
        logMessage(tr("Получили команду возвращения на базу: проблемы с транспортом, отмена"));
        return false;
    }

    auto returnToHomeCommand = qSharedPointerObjectCast<ReturnToHomeOutgoingCommand>(command);
    if (!returnToHomeCommand) {
        logMessage(tr("Получили команду возвращения на базу: внутренняя проблема (ошибка приведения типа), отмена"));
        return false;
    }
    mClient = mavlinkClient;
    MavLinkCommandCreator commandCreator;
    commandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    auto cmd = commandCreator.getNavReturnToLaunchCommand(false);
    mavlinkClient->sendCommand(cmd);

    logMessage(tr("Посылаем команду возвращения на базу"));

    mCurrentRetry = maxRetries;
    mCurrentCmd = command;
    mCommandTimeoutTimer.start();
    return true;
}

bool ReturnToHomeOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    if (mCurrentCmd && command->msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
        mavlink_command_ack_t cmd;
        mavlink_msg_command_ack_decode(&command->msg, &cmd);
        if (cmd.command == MAV_CMD_NAV_RETURN_TO_LAUNCH) {
            qDebug() << Q_FUNC_INFO << cmd.result;
            mCommandTimeoutTimer.stop();
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

void ReturnToHomeOutgoingCommandHandler::onCommandTimeout()
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
