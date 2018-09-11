#include "mountcontroloutgoingcommandhandler.h"

#include <transport/mavlinkclient.h>

#include <entities/mavlinkcommandcreator.h>

#include <protocol/commands/mountcontroloutgoingcommand.h>

#include <QTimer>
#include <QDebug>

namespace {
constexpr int commandTimeout = 2000;
constexpr int maxRetries = 3;
}

using namespace rsvo::mavlinkprotocol;

MountControlOutgoingCommandHandler::MountControlOutgoingCommandHandler(QObject *parent) :
    IOutgoingCommandHandler(parent)
{
    connect(&mCommandTimeoutTimer, &QTimer::timeout,
            this, &MountControlOutgoingCommandHandler::onCommandTimeout);
    mCommandTimeoutTimer.setInterval(commandTimeout);
    mCommandTimeoutTimer.setSingleShot(true);
}

IOutgoingCommand::Command MountControlOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::MountControl;
}

bool MountControlOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
    auto mountControlCommand = qSharedPointerObjectCast<MountControlOutgoingCommand>(command);
    if (!mountControlCommand) {
        logMessage(tr("Получили команду управления подвесом: внутренняя проблема (ошибка приведения типа), отмена"));
        return false;
    }
    mLastPitch = mountControlCommand->pitch();
    mLastCourse = mountControlCommand->pitch();

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
        logMessage(tr("Получили команду управления подвесом: проблемы с транспортом, отмена"));
        return false;
    }

    mCurrentCourse = mLastCourse;
    mCurrentPitch = mLastPitch;

    mClient = mavlinkClient;
    MavLinkCommandCreator commandCreator;
    commandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    auto cmd = commandCreator.getDoMountControlCommand(mountControlCommand->pitch(), mountControlCommand->course());
    mavlinkClient->sendCommand(cmd);
    mCurrentRetry = maxRetries;
    mCurrentCmd = command;
    mCommandTimeoutTimer.start();

    logMessage(tr("Посылаем команду управления подвесом (MAV_CMD_DO_MOUNT_CONTROL): course: %0, pitch: %1")
               .arg(static_cast<double>(mountControlCommand->course()))
               .arg(static_cast<double>(mountControlCommand->pitch())));

    return true;
}

bool MountControlOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    if (mCurrentCmd && command->msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
        mavlink_command_ack_t cmd;
        mavlink_msg_command_ack_decode(&command->msg, &cmd);
        if (cmd.command == MAV_CMD_DO_MOUNT_CONTROL) {
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

void MountControlOutgoingCommandHandler::onCommandTimeout()
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
