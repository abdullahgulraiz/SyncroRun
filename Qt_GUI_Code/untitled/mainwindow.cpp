#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    myPlayer = new Player();
    QObject::connect(myPlayer, SIGNAL(processedImage(QImage, QMap<String, double>)),this, SLOT(updatePlayerUI(QImage,QMap<String, double>)));
    ui->setupUi(this);
    resetPlots();
    initializeSerial("/dev/ttyUSB0"); //CHANGE THIS TO ARDUINO SERIAL PORT
}

void MainWindow::updatePlayerUI(QImage img, QMap<String, double> var)
{
    if (!img.isNull())
    {
        ui->label->setAlignment(Qt::AlignCenter);
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(ui->label->size(),Qt::KeepAspectRatio, Qt::FastTransformation));
        userVariable = var;
        updatePlots(var);
    }
}

MainWindow::~MainWindow()
{
    delete myPlayer;
    delete ui;
}

void MainWindow::loadVideo() {
    if (!filename.isEmpty()){
        if (!myPlayer->loadVideo(filename.toLatin1().data()))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Error");
            msgBox.setText("The selected video could not be opened.");
            msgBox.exec();
        }
        this->setWindowTitle(filename.toLatin1().data());
    }
}

void MainWindow::on_btn_play_clicked()
{
    playVideo();
}

void MainWindow::playVideo() {
    if (filename.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("No video loaded");
        msgBox.exec();
        return;
    }
    if (myPlayer->isStopped())
    {
        myPlayer->Play();
        ui->btn_play->setText(tr("Stop"));
        ui->btn_reset->setEnabled(false);
    }else
    {
        myPlayer->Stop();
        ui->btn_play->setText(tr("Play"));
        ui->btn_reset->setEnabled(true);
    }
}

void MainWindow::on_btn_reset_clicked()
{
    loadVideo();
    resetPlots();
    playVideo();
}

void MainWindow::resetPlots() {
    userVariables.clear();
    totalMaximas = 0;
    clearPlots();
}

void MainWindow::updatePlots(QMap<String, double> var) {
    saveUserVariables();
    rescaleAxes();
    //qDebug() << var["centerXm"] << "," << var["centerYm"];
    ui->plot_position->graph(0)->addData(var["centerXm"], var["centerYm"]);
    ui->plot_position->replot();
    ui->plot_positionTime->graph(0)->addData(var["posTime"], var["centerXm"]);
    ui->plot_positionTime->graph(1)->addData(var["posTime"], var["centerYm"]);
    ui->plot_positionTime->replot();
    ui->plot_velocityTime->graph(0)->addData(var["posTime"], var["vel"]);
    ui->plot_velocityTime->graph(1)->addData(var["posTime"], var["vx"]);
    ui->plot_velocityTime->graph(2)->addData(var["posTime"], var["vy"]);
    ui->plot_velocityTime->replot();
    //send signal of maxima to arduino serially if maxima exists
    if (var["IsMaxima"]) {
        totalMaximas++;
        qDebug() << totalMaximas;
        sendSerial("1");
    }
}

void MainWindow::rescaleAxes() {
    ui->plot_position->xAxis->setRange(*min_element(userVariables["centerXm"].constBegin(), userVariables["centerXm"].constEnd()),
            *max_element(userVariables["centerXm"].constBegin(), userVariables["centerXm"].constEnd()));
    ui->plot_position->yAxis->setRange(*min_element(userVariables["centerYm"].constBegin(), userVariables["centerYm"].constEnd()),
            *max_element(userVariables["centerYm"].constBegin(), userVariables["centerYm"].constEnd()));
    ui->plot_positionTime->xAxis->setRange(*min_element(userVariables["posTime"].constBegin(), userVariables["posTime"].constEnd()),
            *max_element(userVariables["posTime"].constBegin(), userVariables["posTime"].constEnd()));
    double min = *min_element(userVariables["centerYm"].constBegin(), userVariables["centerYm"].constEnd());
    double current = *min_element(userVariables["centerXm"].constBegin(), userVariables["centerXm"].constEnd());
    if (current < min) { min = current; }
    double max = *max_element(userVariables["centerYm"].constBegin(), userVariables["centerYm"].constEnd());
    current = *max_element(userVariables["centerXm"].constBegin(), userVariables["centerXm"].constEnd());
    if (current > max) { max = current; }
    ui->plot_positionTime->yAxis->setRange(min,max);
    ui->plot_velocityTime->xAxis->setRange(*min_element(userVariables["posTime"].constBegin(), userVariables["posTime"].constEnd()),
            *max_element(userVariables["posTime"].constBegin(), userVariables["posTime"].constEnd()));
    min = *min_element(userVariables["vel"].constBegin() + 1, userVariables["vel"].constEnd());
    double currentMin = *min_element(userVariables["vx"].constBegin() + 1, userVariables["vx"].constEnd());
    if (currentMin < min) { min = currentMin; }
    currentMin = *min_element(userVariables["vy"].constBegin() + 1, userVariables["vy"].constEnd());
    if (currentMin < min) { min = currentMin; }
    max = *max_element(userVariables["vel"].constBegin() + 1, userVariables["vel"].constEnd());
    double currentMax = *max_element(userVariables["vx"].constBegin() + 1, userVariables["vx"].constEnd());
    if (currentMax > max) { max = currentMax; }
    currentMax = *max_element(userVariables["vy"].constBegin() + 1, userVariables["vy"].constEnd());
    if (currentMax > max) { max = currentMax; }
    ui->plot_velocityTime->yAxis->setRange(min,max);
//    ui->plot_velocityTime->yAxis->setRange(*min_element(userVariables["vel"].constBegin() + 1, userVariables["vel"].constEnd()),
//            *max_element(userVariables["vel"].constBegin() + 1, userVariables["vel"].constEnd()));
}

void MainWindow::saveUserVariables()
{
    userVariables["posTime"].push_back(userVariable["posTime"]);
    userVariables["centerXm"].push_back(userVariable["centerXm"]);
    userVariables["centerYm"].push_back(userVariable["centerYm"]);
    userVariables["vel"].push_back(userVariable["vel"]);
    userVariables["vx"].push_back(userVariable["vx"]);
    userVariables["vy"].push_back(userVariable["vy"]);
    userVariables["centerError"].push_back(userVariable["centerError"]);
    userVariables["BoundingBox_X"].push_back(userVariable["BoundingBox_X"]);
    userVariables["BoundingBox_Y"].push_back(userVariable["BoundingBox_Y"]);
    userVariables["BoundingBox_Width"].push_back(userVariable["BoundingBox_Width"]);
    userVariables["BoundingBox_Height"].push_back(userVariable["BoundingBox_Height"]);
    userVariables["Detector_BoundingBox_X"].push_back(userVariable["Detector_BoundingBox_X"]);
    userVariables["Detector_BoundingBox_Y"].push_back(userVariable["Detector_BoundingBox_Y"]);
    userVariables["Detector_BoundingBox_Width"].push_back(userVariable["Detector_BoundingBox_Width"]);
    userVariables["Detector_BoundingBox_Height"].push_back(userVariable["Detector_BoundingBox_Height"]);
    userVariables["Angle"].push_back(userVariable["Angle"]);
    userVariables["IsMaxima"].push_back(userVariable["IsMaxima"]);
}

void MainWindow::testPlots() {
    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    ui->plot_position->addGraph();
    ui->plot_position->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->plot_position->xAxis->setLabel("x");
    ui->plot_position->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->plot_position->xAxis->setRange(-1, 1);
    ui->plot_position->yAxis->setRange(0, 1);
    ui->plot_position->replot();

    ui->plot_positionTime->addGraph();
    ui->plot_positionTime->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->plot_positionTime->xAxis->setLabel("x");
    ui->plot_positionTime->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->plot_positionTime->xAxis->setRange(-1, 1);
    ui->plot_positionTime->yAxis->setRange(0, 1);
    ui->plot_positionTime->replot();

    ui->plot_velocityTime->addGraph();
    ui->plot_velocityTime->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->plot_velocityTime->xAxis->setLabel("x");
    ui->plot_velocityTime->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->plot_velocityTime->xAxis->setRange(-1, 1);
    ui->plot_velocityTime->yAxis->setRange(0, 1);
    ui->plot_velocityTime->replot();

}

void MainWindow::on_actionLoad_triggered()
{
   filename = QFileDialog::getOpenFileName(this,
                                         tr("Open Video"), ".",
                                         tr("Video Files (*.avi *.mpg *.mp4)"));
    loadVideo();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionPlay_triggered()
{
    playVideo();
}

void MainWindow::on_actionReset_triggered()
{
    if (myPlayer->isStopped()) {
        loadVideo();
        resetPlots();
        playVideo();
    }
}

void MainWindow::clearPlots() {
    ui->plot_position->clearGraphs();
    ui->plot_position->addGraph();
    ui->plot_position->xAxis->setLabel("CenterX (m)");
    ui->plot_position->yAxis->setLabel("CenterY (m)");
    ui->plot_position->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->plot_position->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot_position->replot();

    ui->plot_positionTime->clearGraphs();
    ui->plot_positionTime->addGraph()->setName("CenterX");
    ui->plot_positionTime->addGraph()->setName("CenterY");
    ui->plot_positionTime->graph(1)->setPen(QPen(QColor(255, 110, 40)));
    ui->plot_positionTime->xAxis->setLabel("Time (s)");
    ui->plot_positionTime->yAxis->setLabel("Position (m)");
    ui->plot_positionTime->legend->setVisible(true);
    //To set legend below graph
    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->plot_positionTime->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(5,0,5,5));
    subLayout->addElement(0,0,ui->plot_positionTime->legend);
    ui->plot_positionTime->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->plot_positionTime->plotLayout()->setRowStretchFactor(1,0.001);
    ui->plot_positionTime->replot();

    ui->plot_velocityTime->clearGraphs();
    ui->plot_velocityTime->addGraph()->setName("Velocity");
    ui->plot_velocityTime->addGraph()->setName("Vx");
    ui->plot_velocityTime->addGraph()->setName("Vy");
    ui->plot_velocityTime->graph(1)->setPen(QPen(QColor(255, 110, 40)));
    ui->plot_velocityTime->graph(2)->setPen(QPen(QColor(34, 139, 34)));
    ui->plot_velocityTime->xAxis->setLabel("Time (s)");
    ui->plot_velocityTime->yAxis->setLabel("Velocity (m/s)");
    ui->plot_velocityTime->legend->setVisible(true);
    //To set legend below graph
    QCPLayoutGrid *subLayout2 = new QCPLayoutGrid;
    ui->plot_velocityTime->plotLayout()->addElement(1,0,subLayout2);
    subLayout2->setMargins(QMargins(5,0,5,5));
    subLayout2->clear();
    subLayout2->addElement(0,0,ui->plot_velocityTime->legend);
    ui->plot_velocityTime->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->plot_velocityTime->plotLayout()->setRowStretchFactor(1,0.001);
    ui->plot_velocityTime->replot();
}

void MainWindow::on_btn_clearPlots_clicked()
{
    clearPlots();
}

void MainWindow::replotAll() {
    if (userVariables.isEmpty()) {
        return;
    }
    clearPlots();
    rescaleAxes();

    ui->plot_position->graph(0)->setData(userVariables["centerXm"], userVariables["centerYm"]);
    ui->plot_position->replot();
    ui->plot_positionTime->graph(0)->setData(userVariables["posTime"], userVariables["centerXm"]);
    ui->plot_positionTime->graph(1)->setData(userVariables["posTime"], userVariables["centerYm"]);
    ui->plot_positionTime->replot();
    ui->plot_velocityTime->graph(0)->setData(userVariables["posTime"], userVariables["vel"]);
    ui->plot_velocityTime->graph(1)->setData(userVariables["posTime"], userVariables["vx"]);
    ui->plot_velocityTime->graph(2)->setData(userVariables["posTime"], userVariables["vy"]);
    ui->plot_velocityTime->replot();
}

void MainWindow::savePlotCSV(int plot)
{
    if (userVariables.isEmpty()) {
        return;
    }
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save CSV"), ".", tr("CSV File (.csv)"));
    QFile data(saveFileName + ".csv");
    if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
        return;
    }
    QTextStream out(&data);
    switch (plot) {
        case 1:
            for (int i = 0; i < userVariables["centerXm"].size(); i++) {
                out << userVariables["centerXm"][i] << "," << userVariables["centerYm"][i] << endl;
            }
            break;
        case 2:
            for (int i = 0; i < userVariables["centerXm"].size(); i++) {
                out << userVariables["posTime"][i] << "," << userVariables["centerXm"][i] << "," << userVariables["centerYm"][i] << endl;
            }
            break;
        case 3:
            for (int i = 0; i < userVariables["centerXm"].size(); i++) {
                out << userVariables["posTime"][i] << "," << userVariables["vel"][i] << "," << userVariables["vx"][i] << "," << userVariables["vy"][i] << endl;
            }
            break;
        case 4:
            for (int i = 0; i < userVariables["centerXm"].size(); i++) {
                out << userVariables["posTime"][i] << "," << userVariables["centerError"][i] << endl;
            }
            break;
        case 5:
            for (int i = 0; i < userVariables["centerXm"].size(); i++) {
                out << userVariables["posTime"][i] << "," << userVariables["BoundingBox_X"][i] << "," << userVariables["BoundingBox_Y"][i] << "," << userVariables["BoundingBox_Width"][i] << "," << userVariables["BoundingBox_Height"][i] << endl;
            }
            break;
        case 6:
            for (int i = 0; i < userVariables["centerXm"].size(); i++) {
                out << userVariables["posTime"][i] << "," << userVariables["Angle"][i] << endl;
            }
            break;
        case 7:
        for (int i = 0; i < userVariables["centerXm"].size(); i++) {
            out << userVariables["posTime"][i] << "," << userVariables["Detector_BoundingBox_X"][i] << "," << userVariables["Detector_BoundingBox_Y"][i] << "," << userVariables["Detector_BoundingBox_Width"][i] << "," << userVariables["Detector_BoundingBox_Height"][i] << endl;
        }
            break;
        default:
            return;
    }
}

void MainWindow::savePlotImage(int plot)
{
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("PNG File (.png)"));
    QString fileExt = ".png";
    switch (plot) {
        case 1:
            ui->plot_position->grab().save(saveFileName + fileExt);
            break;
        case 2:
            ui->plot_positionTime->grab().save(saveFileName + fileExt);
            break;
        case 3:
            ui->plot_velocityTime->grab().save(saveFileName + fileExt);
            break;
        default:
            return;
    }
}

void MainWindow::initializeSerial(QString arduinoSerialPort)
{
    if (serialPort.isOpen()) {
        serialPort.close();
    }
    // "/dev/ttyACM1"
    serialPort.setPortName(arduinoSerialPort);
    serialPort.setBaudRate(9600);
    serialPort.open(QIODevice::ReadWrite);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);
    qDebug() << "Serial initialized";
}

void MainWindow::sendSerial(QString Data)
{
    //qDebug() << "Connecting";
        if (!serialPort.isOpen()) {
            if (!serialPort.open(QIODevice::ReadWrite)) {
                QMessageBox *msg;
                msg->setText("Error: " + serialPort.error());
                msg->show();
                return;
            }
        }
        //qDebug() << "Connected";
        QByteArray toSend = Data.toLocal8Bit();
        //qDebug() << "Sending: " <<  Data;
        if (Data.isEmpty()) {
            return;
        }
        if (serialPort.isOpen() && serialPort.isWritable()) {
            serialPort.write(toSend);
            serialPort.flush();
        }
        qDebug() << "Data sent";
//        QByteArray readData = serialPort.readAll();
//        while (serialPort.waitForReadyRead()) { readData.append(serialPort.readAll()); }
//        if (readData.length() > 1) {
//            qDebug() << readData;
//        }
//        else {
//            qDebug() << "Nothing received";
//        }
        serialPort.close();
}

void MainWindow::on_actionReplotAll_triggered()
{
    replotAll();
}

void MainWindow::on_actionClearPlots_triggered()
{
    clearPlots();
}

void MainWindow::on_actionCenterXm_CenterYm_2_triggered()
{
    savePlotCSV(1);
}

void MainWindow::on_actionCenterXm_CenterYm_triggered()
{
    savePlotImage(1);
}

void MainWindow::on_actionTime_CenterXm_CenterYm_2_triggered()
{
    savePlotCSV(2);
}

void MainWindow::on_actionTime_Velocity_2_triggered()
{
    savePlotCSV(3);
}

void MainWindow::on_actionTime_CenterXm_CenterYm_triggered()
{
    savePlotImage(2);
}

void MainWindow::on_actionTime_Velocity_triggered()
{
    savePlotImage(3);
}

void MainWindow::on_actionCenterXm_changed()
{
    if (ui->actionCenterXm->isChecked()) {
        ui->plot_positionTime->graph(0)->setVisible(true);
        ui->plot_positionTime->graph(0)->addToLegend();
    }
    else {
        ui->plot_positionTime->graph(0)->setVisible(false);
        ui->plot_positionTime->graph(0)->removeFromLegend();
    }
    ui->plot_positionTime->replot();
}

void MainWindow::on_actionCenterYm_changed()
{
    if (ui->actionCenterYm->isChecked()) {
        ui->plot_positionTime->graph(1)->setVisible(true);
        ui->plot_positionTime->graph(1)->addToLegend();
    }
    else {
        ui->plot_positionTime->graph(1)->setVisible(false);
        ui->plot_positionTime->graph(1)->removeFromLegend();
    }
    ui->plot_positionTime->replot();
}

void MainWindow::on_actionMagnitude_changed()
{
    if (ui->actionMagnitude->isChecked()) {
        ui->plot_velocityTime->graph(0)->setVisible(true);
        ui->plot_velocityTime->graph(0)->addToLegend();
    }
    else {
        ui->plot_velocityTime->graph(0)->setVisible(false);
        ui->plot_velocityTime->graph(0)->removeFromLegend();
    }
    ui->plot_velocityTime->replot();
}

void MainWindow::on_actionVx_changed()
{
    if (ui->actionVx->isChecked()) {
        ui->plot_velocityTime->graph(1)->setVisible(true);
        ui->plot_velocityTime->graph(1)->addToLegend();
    }
    else {
        ui->plot_velocityTime->graph(1)->setVisible(false);
        ui->plot_velocityTime->graph(1)->removeFromLegend();
    }
    ui->plot_velocityTime->replot();
}

void MainWindow::on_actionVy_changed()
{
    if (ui->actionVy->isChecked()) {
        ui->plot_velocityTime->graph(2)->setVisible(true);
        ui->plot_velocityTime->graph(2)->addToLegend();
    }
    else {
        ui->plot_velocityTime->graph(2)->setVisible(false);
        ui->plot_velocityTime->graph(2)->removeFromLegend();
    }
    ui->plot_velocityTime->replot();
}

void MainWindow::on_actionTime_PositionErrorX_PositionErrorY_triggered()
{
    savePlotCSV(4);
}

void MainWindow::on_actionTime_BoundingBox_triggered()
{
    savePlotCSV(5);
}

void MainWindow::on_actionTime_Angle_triggered()
{
    savePlotCSV(6);
}

void MainWindow::on_actionTime_DetectorBoundingBox_triggered()
{
    savePlotCSV(7);
}

void MainWindow::on_actionSet_USB_Port_triggered()
{
    QString arduinoSerialPort = QInputDialog::getText(0, "Define Serial Port",
                "Enter serial port the mechanism is connected to:");
    initializeSerial(arduinoSerialPort);
}
