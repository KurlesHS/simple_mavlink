#include "startmissionoutgoingcommandhandler.h"

#include <transport/mavlinkclient.h>
#include <entities/mavlinkcommandcreator.h>
#include <protocol/commands/startmissionoutgoingcommand.h>

#include <QTimer>

using namespace rsvo::mavlinkprotocol;

StartMissionOutgoingCommandHandler::StartMissionOutgoingCommandHandler(QObject *parent) :
    IOutgoingCommandHandler(parent)
{

}


IOutgoingCommand::Command rsvo::mavlinkprotocol::StartMissionOutgoingCommandHandler::commandType() const
{
    return IOutgoingCommand::Command::StartMission;

}

bool rsvo::mavlinkprotocol::StartMissionOutgoingCommandHandler::processCommand(IOutgoingCommandSharedPtr command, const uint8_t systemId, const uint8_t componentId, const uint8_t targetSystemId, const uint8_t targetComponentId, MavLinkClient *mavlinkClient)
{
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
    MavLinkCommandCreator commandCreator;
    commandCreator.setCommonParams(systemId, componentId, targetSystemId, targetComponentId);
    auto cmd = commandCreator.getMissionStartCommand(startMissionCommand->from(), startMissionCommand->to());
    mavlinkClient->sendCommand(cmd);
    // имитируем бурную деятельность по доставке команды (на самом деле нет)
    logMessage(tr("Посылаем команду запуска маршрутного задания (item от %0 до %1)")
               .arg(startMissionCommand->from())
               .arg(startMissionCommand->to()));
    QTimer::singleShot(10, command.data(), [startMissionCommand]() {
        startMissionCommand->makeFinished(true);
    });
    return true;
}

bool rsvo::mavlinkprotocol::StartMissionOutgoingCommandHandler::incommingMavlinkCommand(const MavLinkCommandSharedPtr &command)
{
    Q_UNUSED(command)
    return false;
}
