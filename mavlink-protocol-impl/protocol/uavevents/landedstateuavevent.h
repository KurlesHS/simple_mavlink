#pragma once

#include "../iuavevent.h"


namespace rsvo {

namespace mavlinkprotocol {

struct LandedStateUavEvent : public IUavEvent
{
    enum class State : uint16_t {
        Undefinded = 0,
        OnGround = 1,
        InAir = 2,
        TakeOff = 3,
        Landing = 4
    };

    LandedStateUavEvent(const LandedStateUavEvent::State state) : state(state) {}
    // IUavEvent interface
    virtual Type type() const override { return Type::LandedState; }

    State state;
};

}

}
