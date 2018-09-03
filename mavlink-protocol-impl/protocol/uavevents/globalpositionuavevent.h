#pragma once

#include "../iuavevent.h"


namespace rsvo {

namespace mavlinkprotocol {

struct GlobalPositionUavEvent : public IUavEvent
{
    virtual Type type() const override { return Type::GlobalPosition; }
    float lat; /*< Latitude, expressed as degrees * 1E7*/
    float lon; /*< Longitude, expressed as degrees * 1E7*/
    float alt; /*< Altitude in meters, expressed as * 1000 (millimeters), AMSL (not WGS84 - note that virtually all GPS modules provide the AMSL as well)*/
    float relative_alt; /*< Altitude above ground in meters, expressed as * 1000 (millimeters)*/
};

}

}



