#include "player.h"

Player::Player(QObject *parent)
 : QThread(parent)
{
    stop = true;
}

bool Player::loadVideo(String filename) {
    if (capture.isOpened()) {
        mutex.lock();
        capture.release();
        mutex.unlock();
    }
    capture.open(filename);
    if (capture.isOpened())
    {
        frameRate = (int) capture.get(CV_CAP_PROP_FPS);
        resetUserVariables();
        //initializeKalmanDetector();
        return true;
    }
    else
        return false;
}

void Player::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(LowPriority);
    }
}

void Player::run()
{
    int delay = (1000/frameRate);
    while(!stop){
        if (!capture.read(frame))
        {
            stop = true;
            finished = true;
        }
        trackFace(frame);
        //detectKalman(frame);
        if (frame.channels()== 3){
            cv::cvtColor(frame, RGBframe, CV_BGR2RGB);
            img = QImage((const unsigned char*)(RGBframe.data),
                              RGBframe.cols,RGBframe.rows,QImage::Format_RGB888);
        }
        else
        {
            img = QImage((const unsigned char*)(frame.data),
                                 frame.cols,frame.rows,QImage::Format_Indexed8);
        }
        emit processedImage(img, userVariables);
        saveUserVariables();
        this->msleep(delay);
    }
}

Player::~Player()
{
    mutex.lock();
    stop = true;
    capture.release();
    condition.wakeOne();
    mutex.unlock();
    wait();
}

void Player::Stop()
{
    stop = true;
}

void Player::msleep(int ms){
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
}

bool Player::isStopped() const{
    return this->stop;
}

bool Player::isFinished() const{
    return this->finished;
}

int Player::setCalibrationParameters(double fx, double fy, double distance_cp, double U0, double V0) {
    camera.fx = fx;
    camera.fy = fy;
    camera.f = sqrt(pow(fx,2)+pow(fy,2));
    camera.distance_cp = distance_cp;
    camera.U0 = U0;
    camera.V0 = V0;
    camera.isCalibrated = true;
    return 0;
}

int Player::pixel2meter(int pixelx, int pixely, double &meterx, double &metery) {
    if (camera.isCalibrated) {
        meterx = (camera.distance_cp/camera.fx) * ((double)pixelx - camera.U0);
        metery = (camera.distance_cp/camera.fy) * ((double)pixely - camera.V0);
        return 0;
    }
    return 1;
}

void Player::resetUserVariables(bool ResetVariables) {
    QString sPath = QCoreApplication::applicationDirPath().append("/frontalface.xml");
    QFile::copy(":/haarcascade_frontalface_alt.xml" , sPath);
    if (!cascade.load(sPath.toStdString())) {
        QMessageBox Msgbox;
        Msgbox.setText("Failed to load cascade");
        Msgbox.exec();
        return;
    }
    QFile::remove(sPath);
    double distanceUserCamera = 1.4; //Distance between user and camera
    setCalibrationParameters(666.9656,664.5017,distanceUserCamera,0.0,0.0);
    TrackerKCF::Params param;
    param.desc_pca = TrackerKCF::GRAY | TrackerKCF::CN;
    param.desc_npca = 0;
    param.compress_feature = true;
    param.compressed_size = 2;
    tracker = TrackerKCF::create(param);
    while(1) {
        capture.read(frame);
        //detect face for roi
        cvtColor(frame, gray, CV_BGR2GRAY);
        equalizeHist(gray, gray);
        //Detect faces in the small image
        std::vector<Rect> faces;
        cascade.detectMultiScale( gray, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(70, 70), Size(250,250) );
        if (faces.empty()) {
            QMessageBox Msgbox;
            Msgbox.setText("Failed to detect face. Retry?");
            Msgbox.exec();
            return;
        }
        roi = faces.back();
        detector_roi = roi;
        //qDebug() << "X: " << QString::number(roi.x) << "Y: " << QString::number(roi.y) << "W: " << QString::number(roi.width) << "H: " << QString::number(roi.height);
        if(roi.width!=0 && roi.height!=0) {
            roi_initial = roi;
            center_initial.x = cvRound(roi_initial.x + roi_initial.width*0.5);
            center_initial.y = cvRound(roi_initial.y + roi_initial.height*0.5);
            //pixel2meter(frame.size().width - center_initial.x, frame.size().height - center_initial.y, centerXm_initial, centerYm_initial);
            centerXm_initial = frame.size().width - center_initial.x;
            centerYm_initial = frame.size().height - center_initial.y;
            //qDebug() << "Initial Values:\n" << centerXm_initial << '\t' << centerYm_initial;
            framesTracked = 0;
            break;
        }
    }
    // initialize the tracker
    tracker->init(frame,roi);
    fps = capture.get(CV_CAP_PROP_FPS);
    if (ResetVariables) {
        posTime = 0; previous_posTime = 0; centerXm = 0; centerYm = 0; previous_centerXm = 0; previous_centerYm = 0;
        threshold_highX = 0; threshold_lowX = 0; threshold_highY = 0; threshold_lowY = 0;
        isDecreasing = false; faceHeight = 0;
        is_thresholdLowX_calculated = false; is_thresholdHighX_calculated = false; is_thresholdLowY_calculated = false; is_thresholdHighY_calculated = false;
        isMaxima = false; isGoingRight = false; maximaFalseAlarmCount = 0;
    }
    //calculate height of user's face
    if (faceHeight == 0) {
        qDebug() << "calculateFaceHeight";
        faceHeight = (camera.distance_cp * roi.height)/camera.fy;
    }
}

void Player::updateDetectorRoi() {
    //qDebug() << "UpdateDectorRoi()";
    QString sPath = QCoreApplication::applicationDirPath().append("/frontalface.xml");
    QFile::copy(":/haarcascade_frontalface_alt.xml" , sPath);
    if (!cascade.load(sPath.toStdString())) {
        QMessageBox Msgbox;
        Msgbox.setText("Failed to load cascade");
        Msgbox.exec();
        return;
    }
    QFile::remove(sPath);
    double distanceUserCamera = 1.4; //Distance between user and camera
    setCalibrationParameters(666.9656,664.5017,distanceUserCamera,0.0,0.0);
        capture.read(frame);
        //detect face for roi
        cvtColor(frame, gray, CV_BGR2GRAY);
        equalizeHist(gray, gray);
        //Detect faces in the small image
        std::vector<Rect> faces;
        cascade.detectMultiScale( gray, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(70, 70), Size(250,250) );
        if (faces.empty()) {
            return;
        }
        detector_roi = faces.back();
        //qDebug() << "X: " << QString::number(roi.x) << "Y: " << QString::number(roi.y) << "W: " << QString::number(roi.width) << "H: " << QString::number(roi.height);
        if(roi.width!=0 && roi.height!=0) {
            //qDebug() << "Initial Values:\n" << centerXm_initial << '\t' << centerYm_initial;
            framesTracked = 0;
        }
}

void Player::saveUserVariables()
{
    userVariablesAll["posTime"].push_back(userVariables["posTime"]);
    userVariablesAll["centerXm"].push_back(userVariables["centerXm"]);
    userVariablesAll["centerYm"].push_back(userVariables["centerYm"]);
    userVariablesAll["vel"].push_back(userVariables["vel"]);
    userVariablesAll["vx"].push_back(userVariables["vx"]);
    userVariablesAll["vy"].push_back(userVariables["vy"]);
    userVariablesAll["Angle"].push_back(userVariables["Angle"]);
}

void Player::calculateThresholds() {
    if ((userVariablesAll["centerXm"].size() <= 3) || (is_thresholdLowX_calculated && is_thresholdHighX_calculated && is_thresholdLowY_calculated && is_thresholdHighY_calculated)) {
        qDebug() << "Check 1";
        return;
    }
    int sizeX = userVariablesAll["centerXm"].size();
    if (!isDecreasing && (userVariablesAll["centerXm"][sizeX - 1] - userVariablesAll["centerXm"][sizeX - 2]) < 0) {
        isDecreasing = true;
        qDebug() << "Check 2";
        threshold_lowX = userVariablesAll["centerXm"].takeLast();
        return;
    }
    if (isDecreasing) {
        if (!is_thresholdLowX_calculated || !is_thresholdHighX_calculated) {
            double val = userVariablesAll["centerXm"].takeLast();
            qDebug() << "Val: " << QString::number(val);
            if ((val < threshold_lowX) && !is_thresholdLowX_calculated && !is_thresholdHighX_calculated) { threshold_lowX = val; }
            else if (!is_thresholdLowX_calculated && !is_thresholdHighX_calculated && (val > threshold_lowX)) {
                is_thresholdLowX_calculated = true;
                qDebug() << "Low Threshold X: " << threshold_lowX;
                threshold_highX = val;
            }
            else if ((val > threshold_highX) && is_thresholdLowX_calculated && !is_thresholdHighX_calculated) {
                threshold_highX = val;
            }
            else if (val < threshold_highX && is_thresholdLowX_calculated && !is_thresholdHighX_calculated) {
                qDebug() << "High Threshold X: " << threshold_highX;
                is_thresholdHighX_calculated = true;
            }
        }
    }
}

void Player::thresholdOutputValues() {
    if (is_thresholdLowX_calculated && is_thresholdHighX_calculated) {
        //centerXm = ((centerXm - threshold_lowX)/(threshold_highX - threshold_lowX)) * threshold_highX;
        if (centerXm < threshold_lowX) {
            centerXm = threshold_lowX;
        }
        else if (centerXm > threshold_highX) {
            centerXm = threshold_highX;
        }
    }
}

void Player::trackFace(Mat& frame) {
    timer.start();
    // stop the program if no more images
    if(frame.rows==0 || frame.cols==0) { return; }
    // update the tracking result
    if (!tracker->update(frame,roi)) {
        resetUserVariables(false);
    }
    //updateDetectorRoi();
//    if (framesTracked >= 60) {
//        updateDetectorRoi();
//        framesTracked = -1;
//    }
//    framesTracked++;
    // draw the tracked object and compute center point
    rectangle( frame, roi, Scalar( 255, 0, 0 ), 2, 1 );
    center = Point(-1,-1);
    center.x = cvRound(roi.x + roi.width*0.5);
    center.y = cvRound(roi.y + roi.height*0.5);
    circle(frame, Point(center.x, center.y), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.25), cvRound(roi.y + roi.height*0.25)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.75), cvRound(roi.y + roi.height*0.25)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.25), cvRound(roi.y + roi.height*0.75)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.75), cvRound(roi.y + roi.height*0.75)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    //recalculate depth
    //camera.distance_cp = (faceHeight * camera.fy)/detector_roi.height;
    //qDebug() << camera.distance_cp;
    //qDebug() << "Dist z: " << camera.distance_cp << "\tRoi_w: " << roi.width << "\tRoi_h: " << roi.height << "\tFace height: " << faceHeight;
    //record time
    posTime += (((double)timer.elapsed())/1000.0);
    //convert coordinates to meter
    pixel2meter(frame.size().width - center.x, frame.size().height - center.y, centerXm, centerYm);
    //compute center error in pixels
    double pixelCenterXm, pixelCenterYm, pixelError;
    pixelCenterXm = frame.size().width - center.x;
    pixelCenterYm = frame.size().height - center.y;
    pixelError = sqrt(pow((pixelCenterXm - centerXm_initial),2) + pow((pixelCenterYm - centerYm_initial), 2));
    //qDebug() << posTime << '\t' << centerXm_initial << '\t' << centerYm_initial << '\t' << pixelCenterXm << '\t' << pixelCenterYm << '\t' << (pixelCenterXm - centerXm_initial) << '\t' << (double)(pixelCenterYm - centerYm_initial) << '\t' << pixelError;
    //calculateThresholds();
    //thresholdOutputValues();
    //resolve NaN values when dealing with tanX
    if (std::isnan((centerYm - previous_centerYm)/(centerXm - previous_centerXm)) || std::isinf((centerYm - previous_centerYm)/(centerXm - previous_centerXm))) {
        theta = 90;
    }
    else {
        //theta = 180/3.14*(atan ((centerYm - previous_centerYm)/(centerXm - previous_centerXm)));
        theta = 180/3.14*(atan2 ((centerYm - previous_centerYm),(centerXm - previous_centerXm)));
    }
    //compute velocity
    vel = (sqrt((pow(centerXm - previous_centerXm,2)) + (pow(centerYm - previous_centerYm,2)))/(posTime - previous_posTime));
    //vel = (sqrt((pow(centerXm - previous_centerXm,2)) + (pow(centerYm - previous_centerYm,2)))/fps);
    vx = vel*(sin (theta));
    vy = vel*(cos (theta));
    //detect maxima
    if (centerXm > previous_centerXm && !isGoingRight) {
        isGoingRight = true;
    }
    else if (isGoingRight && centerXm < previous_centerXm) {
        maximaFalseAlarmCount++;
        if (maximaFalseAlarmCount >= 3) {
            isGoingRight = false;
            isMaxima = true;
            maximaFalseAlarmCount = 0;
        }
        //qDebug() << "Maxima Reached";
    }
    //add calculated data to array for transferring it to the other thread for plotting
    userVariables.clear();
    userVariables.insert("posTime", posTime);
    userVariables.insert("centerXm", centerXm);
    userVariables.insert("centerYm", centerYm);
    userVariables.insert("vel", vel);
    userVariables.insert("vx", vx);
    userVariables.insert("vy", vy);
    userVariables.insert("centerError", pixelError);
    userVariables.insert("BoundingBox_X", roi.x);
    userVariables.insert("BoundingBox_Y", roi.y);
    userVariables.insert("BoundingBox_Width", roi.width);
    userVariables.insert("BoundingBox_Height", roi.height);
    userVariables.insert("Detector_BoundingBox_X", detector_roi.x);
    userVariables.insert("Detector_BoundingBox_Y", detector_roi.y);
    userVariables.insert("Detector_BoundingBox_Width", detector_roi.width);
    userVariables.insert("Detector_BoundingBox_Height", detector_roi.height);
    userVariables.insert("Angle", theta);
    userVariables.insert("IsMaxima", isMaxima);
    //discard maxima recognition once value sent
    if (isMaxima) {
        isMaxima = false;
    }
    //store variables for calculations in next iteration
    previous_centerXm = centerXm;
    previous_centerYm = centerYm;
    previous_posTime = posTime;
    //qDebug() << "trackFace() finished";
}

void Player::initializeKalmanDetector(bool ResetVariables) {
    qDebug() << "initializeKalmanDetector()";
    QString sPath = QCoreApplication::applicationDirPath().append("/frontalface.xml");
    QFile::copy(":/haarcascade_frontalface_alt.xml" , sPath);
    if (!cascade.load(sPath.toStdString())) {
        QMessageBox Msgbox;
        Msgbox.setText("Failed to load cascade");
        Msgbox.exec();
        return;
    }
    QFile::remove(sPath);
    double distanceUserCamera = 1.4; //Distance between user and camera
    setCalibrationParameters(666.9656,664.5017,distanceUserCamera,0.0,0.0);
    while(1) {
        capture.read(frame);
        //detect face for roi
        cvtColor(frame, gray, CV_BGR2GRAY);
        equalizeHist(gray, gray);
        //Detect faces in the small image
        std::vector<Rect> faces;
        cascade.detectMultiScale( gray, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(70, 70), Size(250,250) );
        if (faces.empty()) {
            QMessageBox Msgbox;
            Msgbox.setText("Failed to detect face. Retry?");
            Msgbox.exec();
            return;
        }
        roi = faces.back();
        detector_roi = roi;
        //qDebug() << "X: " << QString::number(roi.x) << "Y: " << QString::number(roi.y) << "W: " << QString::number(roi.width) << "H: " << QString::number(roi.height);
        if(roi.width!=0 && roi.height!=0) {
            roi_initial = roi;
            center_initial.x = cvRound(roi_initial.x + roi_initial.width*0.5);
            center_initial.y = cvRound(roi_initial.y + roi_initial.height*0.5);
            pixel2meter(frame.size().width - center_initial.x, frame.size().height - center_initial.y, centerXmeters_initial, centerYmeters_initial);
            centerXm_initial = frame.size().width - center_initial.x;
            centerYm_initial = frame.size().height - center_initial.y;
            //qDebug() << "Initial Values:\n" << centerXm_initial << '\t' << centerYm_initial;
            framesTracked = 0;
            break;
        }
    }
    fps = capture.get(CV_CAP_PROP_FPS);
    if (ResetVariables) {
        posTime = 0; previous_posTime = 0; centerXm = 0; centerYm = 0; previous_centerXm = 0; previous_centerYm = 0;
        threshold_highX = 0; threshold_lowX = 0; threshold_highY = 0; threshold_lowY = 0;
        isDecreasing = false; faceHeight = 0;
        is_thresholdLowX_calculated = false; is_thresholdHighX_calculated = false; is_thresholdLowY_calculated = false; is_thresholdHighY_calculated = false;
    }
    //calculate height of user's face
    if (faceHeight == 0) {
        qDebug() << "calculateFaceHeight";
        faceHeight = (camera.distance_cp * roi.height)/camera.fy;
    }
    kf.init(4,2,0);
    kf.transitionMatrix = (Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    kf_measurement.create(2,1);
    kf_measurement.setTo(Scalar(0));
    kf.statePre.at<float>(0) = centerXmeters_initial;
    kf.statePre.at<float>(1) = centerYmeters_initial;
    kf.statePre.at<float>(2) = 0;
    kf.statePre.at<float>(3) = 0;
    setIdentity(kf.measurementMatrix);
    setIdentity(kf.processNoiseCov, Scalar::all(1e-4));
    setIdentity(kf.measurementNoiseCov, Scalar::all(10));
    setIdentity(kf.errorCovPost, Scalar::all(.1));
//    setIdentity(kf.controlMatrix);
}

void Player::detectKalman(Mat& frame) {
    timer.start();
    // stop the program if no more images
    if(frame.rows==0 || frame.cols==0) { return; }
    // update the tracking result
    Mat prediction;
    prediction = kf.predict();
    Point predictedPt(prediction.at<float>(0),prediction.at<float>(1));
    cvtColor(frame, gray, CV_BGR2GRAY);
    equalizeHist(gray, gray);
    //Detect faces in the small image
    std::vector<Rect> faces;
    cascade.detectMultiScale( gray, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(70, 70), Size(250,250) );
    if (!faces.empty()) {
        roi = faces.back();
    }
    // draw the tracked object and compute center point
    rectangle( frame, roi, Scalar( 255, 0, 0 ), 2, 1 );
    center = Point(-1,-1);
    center.x = cvRound(roi.x + roi.width*0.5);
    center.y = cvRound(roi.y + roi.height*0.5);
    circle(frame, Point(center.x, center.y), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.25), cvRound(roi.y + roi.height*0.25)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.75), cvRound(roi.y + roi.height*0.25)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.25), cvRound(roi.y + roi.height*0.75)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    circle(frame, Point(cvRound(roi.x + roi.width*0.75), cvRound(roi.y + roi.height*0.75)), 2, Scalar( 255, 0, 0 ), 3, 8, 0);
    //recalculate depth
//    camera.distance_cp = (faceHeight * camera.fy)/detector_roi.height;
//    qDebug() << "Dist z: " << camera.distance_cp << "\tRoi_w: " << roi.width << "\tRoi_h: " << roi.height << "\tFace height: " << faceHeight;
    //record time
    posTime += (((double)timer.elapsed())/1000.0);
    //convert coordinates to meter
    pixel2meter(frame.size().width - center.x, frame.size().height - center.y, centerXm, centerYm);
    //kalman filter shiz
    kf_measurement(0) = centerXm;
    kf_measurement(1) = centerYm;
    qDebug() << "Before correction: CenterXm: " << centerXm << " CenterYm: " << centerYm;
    Mat estimated = kf.correct(kf_measurement);
    centerXm = estimated.at<double>(0);
    centerYm = estimated.at<double>(1);
    qDebug() << "After correction: CenterXm: " << centerXm << " CenterYm: " << centerYm;
    //compute center error in pixels
    double pixelCenterXm, pixelCenterYm, pixelError;
    pixelCenterXm = frame.size().width - center.x;
    pixelCenterYm = frame.size().height - center.y;
    pixelError = sqrt(pow((pixelCenterXm - centerXm_initial),2) + pow((pixelCenterYm - centerYm_initial), 2));
    //qDebug() << posTime << '\t' << centerXm_initial << '\t' << centerYm_initial << '\t' << pixelCenterXm << '\t' << pixelCenterYm << '\t' << (pixelCenterXm - centerXm_initial) << '\t' << (double)(pixelCenterYm - centerYm_initial) << '\t' << pixelError;
    //calculateThresholds();
    //thresholdOutputValues();
    //resolve NaN values when dealing with tanX
    if (std::isnan((centerYm - previous_centerYm)/(centerXm - previous_centerXm)) || std::isinf((centerYm - previous_centerYm)/(centerXm - previous_centerXm))) {
        theta = 90;
    }
    else {
        theta = 180/3.14*(atan ((centerYm - previous_centerYm)/(centerXm - previous_centerXm)));
    }
    //compute velocity
    vel = (sqrt((pow(centerXm - previous_centerXm,2)) + (pow(centerYm - previous_centerYm,2)))/(posTime - previous_posTime));
    //vel = (sqrt((pow(centerXm - previous_centerXm,2)) + (pow(centerYm - previous_centerYm,2)))/fps);
    vx = vel*(sin (theta));
    vy = vel*(cos (theta));
    //add calculated data to array for transferring it to the other thread for plotting
    userVariables.clear();
    userVariables.insert("posTime", posTime);
    userVariables.insert("centerXm", centerXm);
    userVariables.insert("centerYm", centerYm);
    userVariables.insert("vel", vel);
    userVariables.insert("vx", vx);
    userVariables.insert("vy", vy);
    userVariables.insert("centerError", pixelError);
    userVariables.insert("BoundingBox_X", roi.x);
    userVariables.insert("BoundingBox_Y", roi.y);
    userVariables.insert("BoundingBox_Width", roi.width);
    userVariables.insert("BoundingBox_Height", roi.height);
    userVariables.insert("Angle", theta);
    //store variables for calculations in next iteration
    previous_centerXm = centerXm;
    previous_centerYm = centerYm;
    previous_posTime = posTime;
}
