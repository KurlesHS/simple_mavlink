#pragma once

#include <protocol/ioutgoingcommand.h>

namespace rsvo {

namespace mavlinkprotocol {

struct IMissionItem {
    enum class Type {
        Waypoint
    };

    virtual ~IMissionItem() = default;
    virtual Type type() = 0;
};

struct WaypointMissionItem : IMissionItem {
    virtual Type type() override;
    qreal longitude;
    qreal latitude;
    qreal altitude;
    qreal delay;
};

using IMissionItemSharedPtr = QSharedPointer<IMissionItem>;

class UploadMissionOutgoingCommandPrivate;

class UploadMissionOutgoingCommand : public IOutgoingCommand
{
    Q_OBJECT
public:
    UploadMissionOutgoingCommand(QObject *parent = nullptr);
    ~UploadMissionOutgoingCommand() override;

    // IOutgoingCommand interface
public:
    virtual Command commandType() const override;
    void setMissionItems(QVector<IMissionItemSharedPtr> missionItems);
    void addMissionItem(const IMissionItemSharedPtr &missionItem);
    void clearMission();
    const QVector<IMissionItemSharedPtr> &missionItems() const;

private:
    UploadMissionOutgoingCommandPrivate *d;
};

}

}
