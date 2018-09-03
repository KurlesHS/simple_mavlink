#pragma once

#include <protocol/ioutgoingcommand.h>

namespace rsvo {

namespace mavlinkprotocol {

class StartMissionOutgoingCommand : public IOutgoingCommand
{
    Q_OBJECT
public:
    StartMissionOutgoingCommand(const uint16_t from, const uint16_t to, QObject *parent=nullptr);

    uint16_t from() const;
    uint16_t to() const;

    // IOutgoingCommand interface
public:
    virtual Command commandType() const override;

private:
    const uint16_t mFrom;
    const uint16_t mTo;
};

}

}
