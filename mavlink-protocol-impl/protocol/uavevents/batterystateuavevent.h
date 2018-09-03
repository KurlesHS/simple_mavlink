#pragma once

#include "../iuavevent.h"


namespace rsvo {

namespace mavlinkprotocol {

struct BatteryStateUavEvent : public IUavEvent
{
    virtual Type type() const override { return Type::BatteryState; }
    int32_t currentConsumed; /*< Consumed charge, in milliampere hours (1 = 1 mAh), -1: autopilot does not provide mAh consumption estimate*/
    int32_t energyConsumed; /*< Consumed energy, in HectoJoules (intergrated U*I*dt)  (1 = 100 Joule), -1: autopilot does not provide energy consumption estimate*/
    int16_t temperature; /*< Temperature of the battery in centi-degrees celsius. INT16_MAX for unknown temperature.*/
    uint16_t voltages[10]; /*< Battery voltage of cells, in millivolts (1 = 1 millivolt). Cells above the valid cell count for this battery should have the UINT16_MAX value.*/
    int16_t currentBattery; /*< Battery current, in 10*milliamperes (1 = 10 milliampere), -1: autopilot does not measure the current*/
    uint8_t id; /*< Battery ID*/
    uint8_t batteryFunction; /*< Function of the battery*/
    uint8_t typeChemistry; /*< Type (chemistry) of the battery*/
    int8_t batteryRemaining; /*< Remaining battery energy: (0%: 0, 100%: 100), -1: autopilot does not estimate the remaining battery*/
};

}

}
