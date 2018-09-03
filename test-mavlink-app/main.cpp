#include "testmavlinkappmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestMavlinkAppMainWindow w;
    w.show();

    return QApplication::exec();
}
