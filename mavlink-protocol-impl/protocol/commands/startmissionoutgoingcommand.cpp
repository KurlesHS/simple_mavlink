#include "startmissionoutgoingcommand.h"

using namespace rsvo::mavlinkprotocol;

StartMissionOutgoingCommand::StartMissionOutgoingCommand(const uint16_t from, const uint16_t to, QObject *parent) :
    IOutgoingCommand (parent),
    mFrom(from),
    mTo(to)
{

}


IOutgoingCommand::Command rsvo::mavlinkprotocol::StartMissionOutgoingCommand::commandType() const
{
    return Command::StartMission;
}

uint16_t StartMissionOutgoingCommand::from() const
{
    return mFrom;
}

uint16_t StartMissionOutgoingCommand::to() const
{
    return mTo;
}
