#include "mainwindow.h"
#include <QApplication>

//typedef QMap<String,double> MyType;
//Q_DECLARE_METATYPE(MyType)

int main(int argc, char *argv[])
{
    qRegisterMetaType<QMap<String, double> >("QMap<String, double>");
    QApplication a(argc, argv);
    MainWindow* w = new MainWindow();
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->setWindowState(Qt::WindowMaximized);
    w->setWindowTitle("SyncroRun GUI");
    w->show();

    return a.exec();
}
