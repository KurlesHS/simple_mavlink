#pragma once

#include <protocol/ioutgoingcommand.h>

namespace rsvo {

namespace mavlinkprotocol {

class ReturnToHomeOutgoingCommand : public IOutgoingCommand
{
    Q_OBJECT
public:
    ReturnToHomeOutgoingCommand(QObject *parent=nullptr);

    // IOutgoingCommand interface
public:
    virtual Command commandType() const override;
};

}

}
