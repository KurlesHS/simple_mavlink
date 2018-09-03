#include "returntohomeoutgoingcommand.h"

using namespace rsvo::mavlinkprotocol;

ReturnToHomeOutgoingCommand::ReturnToHomeOutgoingCommand(QObject *parent) :
    IOutgoingCommand (parent)
{

}

IOutgoingCommand::Command rsvo::mavlinkprotocol::ReturnToHomeOutgoingCommand::commandType() const
{
    return Command::ReturnToHome;
}
