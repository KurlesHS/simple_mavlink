#include "testmavlinkappmainwindow.h"
#include "ui_testmavlinkappmainwindow.h"

#include <protocol/commands/mountcontroloutgoingcommand.h>
#include <protocol/commands/returntohomeoutgoingcommand.h>
#include <protocol/commands/startmissionoutgoingcommand.h>
#include <protocol/commands/uploadmissionoutgoingcommand.h>

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
}

void TestMavlinkAppMainWindow::updatePitchCourseLabel()
{
    ui->labelCurrentPitchCourse->setText(tr("course: %0, pitch: %1").arg(mCourse).arg(mPitch));
}

void TestMavlinkAppMainWindow::correctPitchCourse()
{
    if (mCourse < 0) {
        mCourse = 0;
    } else if (mCourse > 360) {
        mCourse = 360;
    }

    if (mPitch < 0) {
        mPitch = 0;
    } else if (mPitch > 360) {
        mPitch = 360;
    }
}

void TestMavlinkAppMainWindow::sendPitchCourseCmd()
{
    correctPitchCourse();
    updatePitchCourseLabel();
    auto cmd = new MountControlOutgoingCommand(mPitch, mCourse);
    connect(cmd, &MountControlOutgoingCommand::finished,
            this, [this](bool result) {
        qDebug() << "result mount control" << result;
        this->onLog(tr("Result mount control: %0").arg(result));
    });
    mCommandHandler.addCommand("1", IOutgoingCommandSharedPtr(cmd));

}

void TestMavlinkAppMainWindow::on_pushButtonStartMission_clicked()
{
    auto cmd = new StartMissionOutgoingCommand(0, 2);
    connect(cmd, &StartMissionOutgoingCommand::finished,
            this, [this](bool result) {
        qDebug() << "result start mission" << result;
        this->onLog(tr("Result start mission: %0").arg(result));
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
