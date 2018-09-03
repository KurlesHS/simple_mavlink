#ifndef TESTMAVLINKAPPMAINWINDOW_H
#define TESTMAVLINKAPPMAINWINDOW_H

#include <QMainWindow>

#include <protocol/handlers/uploadmissionoutgoingcommandhandler.h>

#include <protocol/mavlinkcommandhandler.h>

#include <protocol/iuavevent.h>

#include <transport/tcpmavlinktransport.h>
#include <transport/mavlinkclient.h>

namespace Ui {
class TestMavlinkAppMainWindow;
}

class TestMavlinkAppMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestMavlinkAppMainWindow(QWidget *parent = nullptr);
    ~TestMavlinkAppMainWindow() override;

private slots:
    void on_pushButtonUploadMission_clicked();
    void on_pushButtonStartMission_clicked();

    void on_pushButtonUp_clicked();

    void on_pushButtonDown_clicked();

    void on_pushButtonRight_clicked();

    void on_pushButtonLeft_clicked();

    void on_pushButtonReturnToHome_clicked();

private:
    void onLog(const QString &msg);
    void onUavEvent(const QString &uavId, const rsvo::mavlinkprotocol::IUavEventSharedPtr &onUavEvent);

    void updatePitchCourseLabel();
    void correctPitchCourse();
    void sendPitchCourseCmd();


private:
    Ui::TestMavlinkAppMainWindow *ui;
    rsvo::mavlinkprotocol::TcpMavlinkTransport mTransport;
    rsvo::mavlinkprotocol::MavLinkClient mClient;
    rsvo::mavlinkprotocol::MavlinkCommandHandler mCommandHandler;
    int mPitch;
    int mCourse;
};

#endif // TESTMAVLINKAPPMAINWINDOW_H
