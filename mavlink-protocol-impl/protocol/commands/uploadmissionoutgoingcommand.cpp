#include "uploadmissionoutgoingcommand.h"

#include <QVector>

using namespace rsvo::mavlinkprotocol;

namespace rsvo {

namespace mavlinkprotocol {
class UploadMissionOutgoingCommandPrivate {
public:
    QVector<IMissionItemSharedPtr> missionItems;
};
}

}

UploadMissionOutgoingCommand::UploadMissionOutgoingCommand(QObject *parent) :
    IOutgoingCommand (parent),
    d(new UploadMissionOutgoingCommandPrivate)
{

}

UploadMissionOutgoingCommand::~UploadMissionOutgoingCommand()
{
    delete d;
}


IOutgoingCommand::Command rsvo::mavlinkprotocol::UploadMissionOutgoingCommand::commandType() const
{
    return IOutgoingCommand::Command::UploadMission;
}

void UploadMissionOutgoingCommand::setMissionItems(QVector<IMissionItemSharedPtr> missionItems)
{
    d->missionItems = missionItems;
}

void UploadMissionOutgoingCommand::addMissionItem(const IMissionItemSharedPtr &missionItem)
{
    d->missionItems.append(missionItem);
}

void UploadMissionOutgoingCommand::clearMission()
{
    d->missionItems.clear();
}

const QVector<IMissionItemSharedPtr> &UploadMissionOutgoingCommand::missionItems() const
{
    return d->missionItems;
}

IMissionItem::Type rsvo::mavlinkprotocol::WaypointMissionItem::type()
{
    return Type::Waypoint;
}
