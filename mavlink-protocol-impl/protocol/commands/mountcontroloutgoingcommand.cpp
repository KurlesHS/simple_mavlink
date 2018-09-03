#include "mountcontroloutgoingcommand.h"

using namespace rsvo::mavlinkprotocol;

MountControlOutgoingCommand::MountControlOutgoingCommand(const float pitch, const float course, QObject *parent) :
    IOutgoingCommand(parent),
    mPitch(pitch),
    mCourse(course)
{

}


IOutgoingCommand::Command rsvo::mavlinkprotocol::MountControlOutgoingCommand::commandType() const
{
    return Command::MountControl;
}

float MountControlOutgoingCommand::pitch() const
{
    return mPitch;
}

float MountControlOutgoingCommand::course() const
{
    return mCourse;
}
