#pragma once

#include <protocol/ioutgoingcommand.h>

namespace rsvo {

namespace mavlinkprotocol {

class MountControlOutgoingCommand : public IOutgoingCommand
{
    Q_OBJECT
public:    
    explicit MountControlOutgoingCommand(const float pitch, const float course, QObject *parent = nullptr);

    float pitch() const;
    float course() const;

    // IOutgoingCommand interface
public:
    virtual Command commandType() const override;

private:
    const float mPitch;
    const float mCourse;
};

}

}
