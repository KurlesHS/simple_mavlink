#include "mountcontroloutgoingcommandhandler.h"

#include <transport/mavlinkclient.h>

#include <entities/mavlinkcommandcreator.h>

#include <protocol/commands/mountcontroloutgoingcommand.h>

#include <QTimer>

using namespace rsvo::mavlinkprotocol;

MountControlOutgoingCommandHandler::MountControlOutgoingCommandHandler(QObject *parent) :
    IOutgoingCommandHandler(parent)
{

}

IOutgoingCommand::Command MountControlOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::MountControl;
}

bool MountControlOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
    if (!mavlinkClient || !mavlinkClient->transport() || !mavlinkClient->transport()->isOpen()) {
        // нет транспорта
        logMessage(tr("Получили команду управления подвесом: проблемы с транспортом, отмена"));
        return false;
    }

    auto mountControlCommand = qSharedPointerObjectCast<MountControlOutgoingCommand>(command);
    if (!mountControlCommand) {
        logMessage(tr("Получили команду управления подвесом: внутренняя проблема (ошибка приведения типа), отмена"));
        return false;
    }
    MavLinkCommandCreator commandCreator;
    commandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    auto cmd = commandCreator.getRcChanneslOverrideCommand(mountControlCommand->pitch(), mountControlCommand->course());
    mavlinkClient->sendCommand(cmd);
    // имитируем бурную деятельность по доставке команды (на самом деле нет)
    logMessage(tr("Посылаем команду управления подвесом (RC_CHANNELS_OVERRIDE): course: %0, pitch: %1")
               .arg(static_cast<double>(mountControlCommand->course()))
               .arg(static_cast<double>(mountControlCommand->pitch())));
    QTimer::singleShot(10, command.data(), [mountControlCommand]() {
        mountControlCommand->makeFinished(true);
    });
    return true;
}

bool MountControlOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    Q_UNUSED(command)
    return false;
}
