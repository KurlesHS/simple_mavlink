INCLUDEPATH *= $$PWD

HEADERS += \
    $$PWD/transport/imavlinktransport.h \
    $$PWD/transport/serialportmavlinktransport.h \
    $$PWD/transport/tcpmavlinktransport.h \
    $$PWD/transport/mavlinkclient.h \
    $$PWD/entities/mavlinkcommand.h \
    $$PWD/entities/mavlinkcommandcreator.h \
    $$PWD/protocol/mavlinkcommandhandler.h \
    $$PWD/protocol/ioutgoingcommand.h \
    $$PWD/protocol/ioutgoingcommandhandler.h \
    $$PWD/protocol/commands/uploadmissionoutgoingcommand.h \
    $$PWD/protocol/handlers/uploadmissionoutgoingcommandhandler.h \
    $$PWD/protocol/iuavevent.h \
    $$PWD/protocol/commands/returntohomeoutgoingcommand.h \
    $$PWD/protocol/commands/startmissionoutgoingcommand.h \
    $$PWD/protocol/commands/mountcontroloutgoingcommand.h \
    $$PWD/protocol/handlers/mountcontroloutgoingcommandhandler.h \
    $$PWD/protocol/handlers/returntohomeoutgoingcommandhandler.h \
    $$PWD/protocol/handlers/startmissionoutgoingcommandhandler.h \
    $$PWD/protocol/uavevents/landedstateuavevent.h \
    $$PWD/protocol/uavevents/altitudeuavevent.h \
    $$PWD/protocol/uavevents/batterystateuavevent.h \
    $$PWD/protocol/uavevents/globalpositionuavevent.h \
    $$PWD/protocol/uavevents/missionitemreacheduavevent.h

SOURCES += \
    $$PWD/transport/imavlinktransport.cpp \
    $$PWD/transport/serialportmavlinktransport.cpp \
    $$PWD/transport/tcpmavlinktransport.cpp \
    $$PWD/transport/mavlinkclient.cpp \
    $$PWD/entities/mavlinkcommandcreator.cpp \
    $$PWD/protocol/mavlinkcommandhandler.cpp \
    $$PWD/protocol/commands/uploadmissionoutgoingcommand.cpp \
    $$PWD/protocol/handlers/uploadmissionoutgoingcommandhandler.cpp \
    $$PWD/protocol/commands/returntohomeoutgoingcommand.cpp \
    $$PWD/protocol/commands/startmissionoutgoingcommand.cpp \
    $$PWD/protocol/commands/mountcontroloutgoingcommand.cpp \
    $$PWD/protocol/handlers/mountcontroloutgoingcommandhandler.cpp \
    $$PWD/protocol/handlers/returntohomeoutgoingcommandhandler.cpp \
    $$PWD/protocol/handlers/startmissionoutgoingcommandhandler.cpp

