#include "returntohomeoutgoingcommandhandler.h"

#include <transport/mavlinkclient.h>
#include <entities/mavlinkcommandcreator.h>
#include <protocol/commands/returntohomeoutgoingcommand.h>

#include <QTimer>

using namespace rsvo::mavlinkprotocol;

ReturnToHomeOutgoingCommandHandler::ReturnToHomeOutgoingCommandHandler(QObject *parent) : IOutgoingCommandHandler(parent)
{

}


IOutgoingCommand::Command ReturnToHomeOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::ReturnToHome;

}

bool ReturnToHomeOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
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
    MavLinkCommandCreator commandCreator;
    commandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    auto cmd = commandCreator.getNavReturnToLaunchCommand(false);
    mavlinkClient->sendCommand(cmd);
    // имитируем бурную деятельность по доставке команды (на самом деле нет)
    logMessage(tr("Посылаем команду возвращения на базу"));
    QTimer::singleShot(10, command.data(), [returnToHomeCommand]() {
        returnToHomeCommand->makeFinished(true);
    });
    return true;
}

bool ReturnToHomeOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    Q_UNUSED(command)
    return false;
}
