#pragma once

#include "../iuavevent.h"


namespace rsvo {

namespace mavlinkprotocol {

struct AltitudeUavEvent : public IUavEvent
{
    // IUavEvent interface
    virtual Type type() const override { return Type::Altitude; }

    float altitudeAmsl;
    float altitudeLocal;
    float altitudeRelative;
    float altitudeTerrain;
};

}

}
