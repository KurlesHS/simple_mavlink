#pragma once

#include <QObject>
#include <QSharedPointer>

namespace rsvo {

namespace mavlinkprotocol {

class IOutgoingCommand;

using IOutgoingCommandSharedPtr = QSharedPointer<IOutgoingCommand>;

class IOutgoingCommand : public QObject
{
    Q_OBJECT
public:
    enum class Command {
        UploadMission,
        StartMission,
        ReturnToHome,
        MountControl
    };
public:
    IOutgoingCommand(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IOutgoingCommand() {}

    virtual Command commandType() const = 0;

    void makeFinished(const bool result) {
        emit finished(result);
    }

signals:
    void finished(const bool result);
};

}

}
