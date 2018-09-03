#include "imavlinktransport.h"

using namespace rsvo::mavlinkprotocol;

IMavlinkTransport::IMavlinkTransport(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<IMavlinkTransport::ConnectionStatus>("rsvo::mavlinkprotocol::IMavlinkTransport::ConnectionStatus");
}
