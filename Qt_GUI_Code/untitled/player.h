#ifndef PLAYER_H
#define PLAYER_H
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QFile>
#include <QCoreApplication>
#include <QMap>
#include <QtDebug>
#include <string.h>
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/tracking.hpp"
using namespace cv;
class Player : public QThread
{    Q_OBJECT

 private:
    bool stop;
    bool finished;
    QMutex mutex;
    QWaitCondition condition;
    Mat frame, gray;
    int frameRate;
    VideoCapture capture;
    Mat RGBframe;
    QImage img;
    QMap<String, double> userVariables;
    QMap<String, QVector<double> > userVariablesAll;

    //NTUST variables
    Rect2d roi, roi_initial, detector_roi;
    Point center, center_initial;
    Ptr<TrackerKCF> tracker;
    CascadeClassifier cascade;
    KalmanFilter kf;
    Mat_<float> kf_measurement;
    struct CAMERA_PARAMETERS {
         double fx;
         double fy;
         double f;
         double distance_cp;
         double U0;
         double V0;
         bool isCalibrated;
     } camera;
     int pixel2meter(int pixelx, int pixely, double &meterx, double &metery);
     int setCalibrationParameters( double fx, double fy, double distance_cp, double U0, double V0);
     void resetUserVariables(bool ResetVariables = true);
     void updateDetectorRoi();
     void trackFace(Mat&);
     void calculateThresholds();
     void saveUserVariables();
     void thresholdOutputValues();
     void initializeKalmanDetector(bool ResetVariables = true);
     void detectKalman(Mat&);
     QElapsedTimer timer;
     double posTime, previous_posTime, previous_centerXm, previous_centerYm, fps, centerXm, centerYm, theta, vel, vx, vy, faceHeight, centerXm_initial, centerYm_initial;
     double error_position_X, error_position_Y, framesTracked, centerXmeters_initial, centerYmeters_initial;
     double threshold_highX, threshold_lowX, threshold_highY, threshold_lowY;
     bool is_thresholdLowX_calculated, is_thresholdHighX_calculated, is_thresholdLowY_calculated, is_thresholdHighY_calculated, isDecreasing, isMaxima, isGoingRight;
     int maximaFalseAlarmCount;
 signals:
 //Signal to output frame to be displayed
      void processedImage(const QImage &image, const QMap<String, double> &userVariables);
 protected:
     void run();
     void msleep(int ms);

 public:
    //Constructor
    Player(QObject *parent = 0);
    //Destructor
    ~Player();
    //Load a video from memory
    bool loadVideo(String filename);
    //Play the video
    void Play();
    //Stop the video
    void Stop();
    //check if the player has been stopped
    bool isStopped() const;
    //check if the video has ended
    bool isFinished() const;
};
#endif // VIDEOPLAYER_H
