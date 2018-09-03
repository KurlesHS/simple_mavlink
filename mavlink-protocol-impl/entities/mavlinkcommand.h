#pragma once

#include <QMetaType>
#include <QSharedPointer>

#include <mavlink/common/mavlink.h>

namespace rsvo {

namespace mavlinkprotocol {

struct MavLinkCommand
{
    mavlink_message_t msg;
};

using MavLinkCommandSharedPtr = QSharedPointer<MavLinkCommand>;

}

}

Q_DECLARE_METATYPE(rsvo::mavlinkprotocol::MavLinkCommandSharedPtr)
