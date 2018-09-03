#pragma once

#include "../iuavevent.h"


namespace rsvo {

namespace mavlinkprotocol {

struct MissionItemReachedUavEvent : public IUavEvent
{
    virtual Type type() const override { return Type::MissionItemReached; }
    uint16_t seq;
};

}

}

