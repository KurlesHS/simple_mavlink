#pragma once

#include <QSharedPointer>

namespace rsvo {

namespace mavlinkprotocol {

struct IUavEvent;

using IUavEventSharedPtr = QSharedPointer<IUavEvent>;

struct IUavEvent
{
    enum class Type {
        LandedState,
        Altitude,
        BatteryState,
        GlobalPosition,
        MissionItemReached
    };

    virtual ~IUavEvent() = default;
    virtual Type type() const = 0;

};

}

}
