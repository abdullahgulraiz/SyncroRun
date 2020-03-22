#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QDialog>
#include <algorithm>
#include <player.h>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    //Display video frame in player UI
    void updatePlayerUI(QImage img, QMap<String, double> var);

    // Slot for the play push button.
    void on_btn_play_clicked();

    void on_btn_reset_clicked();

    void on_actionLoad_triggered();

    void on_actionExit_triggered();

    void on_actionPlay_triggered();

    void on_actionReset_triggered();

    void on_btn_clearPlots_clicked();

    void on_actionReplotAll_triggered();

    void on_actionClearPlots_triggered();

    void on_actionCenterXm_CenterYm_triggered();

    void on_actionCenterXm_CenterYm_2_triggered();

    void on_actionTime_CenterXm_CenterYm_2_triggered();

    void on_actionTime_Velocity_2_triggered();

    void on_actionTime_CenterXm_CenterYm_triggered();

    void on_actionTime_Velocity_triggered();

    void on_actionCenterXm_changed();

    void on_actionCenterYm_changed();

    void on_actionMagnitude_changed();

    void on_actionVx_changed();

    void on_actionVy_changed();

    void on_actionTime_PositionErrorX_PositionErrorY_triggered();

    void on_actionTime_BoundingBox_triggered();

    void on_actionTime_Angle_triggered();

    void on_actionTime_DetectorBoundingBox_triggered();

    void on_actionSet_USB_Port_triggered();

private:
    Ui::MainWindow *ui;
    Player* myPlayer;
    QString filename;
    QMap<String, double> userVariable;
    QMap<String, QVector<double> > userVariables;
    QSerialPort serialPort;
    int totalMaximas;
    void loadVideo();
    void playVideo();
    void testPlots();
    void updatePlots(QMap<String, double>);
    void resetPlots();
    void rescaleAxes();
    void saveUserVariables();
    void clearPlots();
    void replotAll();
    void savePlotCSV(int);
    void savePlotImage(int);
    void initializeSerial(QString);
    void sendSerial(QString Data);
};

#endif // MAINWINDOW_H
