#include "worker.h"

Worker::Worker(QObject *parent) :
    QObject(parent),
    status(false),
    toggleStream(false),
    horseDetect(false)
{
    cap = new cv::VideoCapture();
    haarCascade = cv::CascadeClassifier("/home/darkness/cascade.xml");
}

Worker::~Worker() {
    if(cap->isOpened()) cap->release();
    delete cap;
}

void Worker::receiveGrabFrame() {
    if(!toggleStream) return;

    (*cap) >> _frameOriginal;
    if(_frameOriginal.empty()) return;

    process();

    QImage output((const unsigned char *)_frameOriginal.data, _frameOriginal.cols, _frameOriginal.rows, QImage::Format_RGB888);

    emit sendFrame(output);
}

void Worker::process() {
    cv::cvtColor(_frameOriginal, _frameOriginal, CV_BGR2RGB);

    if (horseDetect) {
        detectAndDisplay(_frameOriginal);
    }
}

void Worker::checkIfDeviceAlredyOpened() {
    if(cap->isOpened()) cap->release();
    cap->open("/home/darkness/file1.avi");
}

void Worker::receiveSetup() {
    checkIfDeviceAlredyOpened();
    if(cap->isOpened()) {
        status = false;
        return;
    }

    status = true;
}

void Worker::detectAndDisplay(cv::Mat image) {
    cv::cvtColor(image, _frameProcessed, CV_RGB2GRAY);

    cv::equalizeHist(_frameProcessed, _frameProcessed);
    haarCascade.detectMultiScale(_frameProcessed, horses, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, cv::Size(300, 300));

    for( size_t i = 0; i < horses.size(); i++ )
      {
        cv::Point center( horses[i].x + horses[i].width*0.5, horses[i].y + horses[i].height*0.5 );
        cv::ellipse(image, center, cv::Size( horses[i].width*0.5, horses[i].height*0.5), 0, 0, 360, cv::Scalar( 0, 139, 139 ), 4, 8, 0 );
      }
}

void Worker::receiveToggleStream() {
    toggleStream = !toggleStream;

}

void Worker::receiveHorseDetect() {
    horseDetect = !horseDetect;
}

