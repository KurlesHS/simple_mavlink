#include "testmavlinkappmainwindow.h"
#include "ui_testmavlinkappmainwindow.h"

#include <protocol/commands/mountcontroloutgoingcommand.h>
#include <protocol/commands/returntohomeoutgoingcommand.h>
#include <protocol/commands/startmissionoutgoingcommand.h>
#include <protocol/commands/uploadmissionoutgoingcommand.h>


#include <protocol/uavevents/altitudeuavevent.h>
#include <protocol/uavevents/batterystateuavevent.h>
#include <protocol/uavevents/globalpositionuavevent.h>
#include <protocol/uavevents/landedstateuavevent.h>
#include <protocol/uavevents/missionitemreacheduavevent.h>

#include <QDateTime>

using namespace rsvo::mavlinkprotocol;

TestMavlinkAppMainWindow::TestMavlinkAppMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestMavlinkAppMainWindow),
    //mTransport("192.168.91.128", 5762),
    mTransport("127.0.0.1", 5762),
    mClient(&mTransport),
    mCommandHandler(&mClient, 255, 0),
    mPitch(0),
    mCourse(0)
{
    ui->setupUi(this);
    connect(&mCommandHandler, &MavlinkCommandHandler::logMessage,
            this, &TestMavlinkAppMainWindow::onLog);
    connect(&mCommandHandler, &MavlinkCommandHandler::uavEvent,
            this, &TestMavlinkAppMainWindow::onUavEvent);

    mCommandHandler.addUav("1", 1, 0);
    mTransport.open();

}

TestMavlinkAppMainWindow::~TestMavlinkAppMainWindow()
{
    delete ui;
}

void TestMavlinkAppMainWindow::on_pushButtonUploadMission_clicked()
{
    UploadMissionOutgoingCommand *cmd = new UploadMissionOutgoingCommand();
    auto item = new WaypointMissionItem();
    item->latitude = 47.3975548;
    item->longitude = 8.5455966;
    item->altitude = 40;
    item->delay = 5;
    cmd->addMissionItem(IMissionItemSharedPtr(item));
    item = new WaypointMissionItem();
    item->latitude = 47.397681;
    item->longitude = 8.5452308;
    item->altitude = 50;
    item->delay = 5;
    cmd->addMissionItem(IMissionItemSharedPtr(item));
    connect(cmd, &UploadMissionOutgoingCommand::finished,
            this, [this](bool result) {
        qDebug() << "result" << result;
        this->onLog(tr("Result: %0").arg(result));
    });
    mCommandHandler.addCommand("1", IOutgoingCommandSharedPtr(cmd));
}

void TestMavlinkAppMainWindow::onLog(const QString &msg)
{
    ui->textEdit->append(tr("%0: %1").arg(QDateTime::currentDateTime().toString(), msg));
}

void TestMavlinkAppMainWindow::onUavEvent(const QString &uavId, const IUavEventSharedPtr &uavEvent)
{

    ui->textEdit->append(tr("Принято событие %0 от БЛА %1")
                         .arg(static_cast<int>(uavEvent->type()))
                         .arg(uavId));

    switch (uavEvent->type()) {
    case IUavEvent::Type::Altitude: {
        auto *ev = dynamic_cast<AltitudeUavEvent*>(uavEvent.data());
        if (ev) {
            ui->textEdit->append(tr(R"(Принято событие ALTITUDE: Amsl: %0, Local: %1, Rel: %2, Terrain: %3)")
                                 .arg(static_cast<double>(ev->altitudeAmsl))
                                 .arg(static_cast<double>(ev->altitudeLocal))
                                 .arg(static_cast<double>(ev->altitudeRelative))
                                 .arg(static_cast<double>(ev->altitudeTerrain)));
        }
    }
        break;
    case IUavEvent::Type::BatteryState: {
        auto *ev = dynamic_cast<BatteryStateUavEvent*>(uavEvent.data());
        if (ev) {
            ui->textEdit->append(tr(R"(Принято событие BatteryState: temperature: %0, remaining: %1)")
                                 .arg(ev->temperature)
                                 .arg(ev->batteryRemaining));
        }

    }
        break;
    case IUavEvent::Type::GlobalPosition: {
        auto *ev = dynamic_cast<GlobalPositionUavEvent*>(uavEvent.data());
        if (ev) {
            ui->textEdit->append(tr(R"(Принято событие GlobalPosition: lat: %0, lon: %1, alt: %2, rel_alt: %3)")
                                 .arg(static_cast<double>(ev->lat))
                                 .arg(static_cast<double>(ev->lon))
                                 .arg(static_cast<double>(ev->alt))
                                 .arg(static_cast<double>(ev->relative_alt)));
        }
    }
        break;
    case IUavEvent::Type::LandedState: {
        auto *ev = dynamic_cast<LandedStateUavEvent*>(uavEvent.data());
        if (ev) {
            ui->textEdit->append(tr(R"(Принято событие LandedState, state: %0)")
                                 .arg(static_cast<int>(ev->state)));
        }
    }
        break;
    case IUavEvent::Type::MissionItemReached: {
        auto *ev = dynamic_cast<MissionItemReachedUavEvent*>(uavEvent.data());
        if (ev) {
            ui->textEdit->append(tr(R"(Принято событие MissionItemReached, seq: %0)")
                                 .arg(static_cast<int>(ev->seq)));
        }
    }
        break;
    }
}

void TestMavlinkAppMainWindow::updatePitchCourseLabel()
{
    ui->labelCurrentPitchCourse->setText(tr("course: %0, pitch: %1").arg(mCourse).arg(mPitch));
}

void TestMavlinkAppMainWindow::correctPitchCourse()
{
    if (mCourse < -180) {
        mCourse = -180;
    } else if (mCourse > 180) {
        mCourse = 180;
    }

    if (mPitch < -180) {
        mPitch = -180;
    } else if (mPitch > 90) {
        mPitch = 90;
    }
}

void TestMavlinkAppMainWindow::sendPitchCourseCmd()
{
    correctPitchCourse();
    updatePitchCourseLabel();
    auto cmd = new MountControlOutgoingCommand(mPitch, mCourse);
    int p = mPitch;
    int c = mCourse;
    connect(cmd, &MountControlOutgoingCommand::finished,
            this, [this, p, c](bool result) {
        qDebug() << "result mount control" << result;
        onLog(tr("Result mount control: %0").arg(result));
        if (result && (p != mPitch || c != mCourse)) {
            this->sendPitchCourseCmd();
        }
    });

    mCommandHandler.addCommand("1", IOutgoingCommandSharedPtr(cmd));
}

void TestMavlinkAppMainWindow::on_pushButtonStartMission_clicked()
{
    auto cmd = new StartMissionOutgoingCommand(0, 2);
    connect(cmd, &StartMissionOutgoingCommand::finished,
            this, [this](bool result) {
        qDebug() << "result start mission" << result;
        onLog(tr("Result start mission: %0").arg(result));
    });
    mCommandHandler.addCommand("1", IOutgoingCommandSharedPtr(cmd));
}

void TestMavlinkAppMainWindow::on_pushButtonUp_clicked()
{
    mPitch += 6;
    sendPitchCourseCmd();
}

void TestMavlinkAppMainWindow::on_pushButtonDown_clicked()
{
    mPitch -= 6;
    sendPitchCourseCmd();
}

void TestMavlinkAppMainWindow::on_pushButtonRight_clicked()
{
    mCourse += 6;
    sendPitchCourseCmd();
}

void TestMavlinkAppMainWindow::on_pushButtonLeft_clicked()
{
    mCourse -= 6;
    sendPitchCourseCmd();
}

void TestMavlinkAppMainWindow::on_pushButtonReturnToHome_clicked()
{
    auto cmd = new ReturnToHomeOutgoingCommand();
    connect(cmd, &ReturnToHomeOutgoingCommand::finished,
            this, [this](bool result) {
        qDebug() << "result return to home" << result;
        this->onLog(tr("Result return to home: %0").arg(result));
    });
    mCommandHandler.addCommand("1", IOutgoingCommandSharedPtr(cmd));
}
