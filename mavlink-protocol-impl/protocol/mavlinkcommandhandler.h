#pragma once

#include <QObject>

#include <transport/mavlinkclient.h>
#include <protocol/iuavevent.h>

#include "ioutgoingcommandhandler.h"
#include <QVariantAnimation>

namespace rsvo {

namespace mavlinkprotocol {

class MavlinkCommandHandlerPrivate;

class MavlinkCommandHandler : public QObject
{
    Q_OBJECT
public:
    explicit MavlinkCommandHandler(MavLinkClient *client, const uint8_t systemId, const uint8_t componentId, QObject *parent = nullptr);

    void registerOutgoingCommandHandler(IOutgoingCommandHandler *handler);
    void addUav(const QString &uavId, const uint8_t systemId, const uint8_t componentId);
    bool addCommand(const QString &uavId, IOutgoingCommandSharedPtr command);

signals:
    void logMessage(const QString &message);
    void uavEvent(const QString &uavId, const IUavEventSharedPtr &uavEvent);
    void linkStatus(const bool linkAccured);

public slots:

private:
    MavlinkCommandHandlerPrivate *d;
};

}

}
