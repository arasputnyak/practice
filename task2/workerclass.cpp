#include "workerclass.h"

WorkerClass::WorkerClass(QObject *parent) :
    QObject(parent),
    status(false),
    //обработка и показ видео
    toggleStream(false),
    //запись видео
    toggleStream2(false),
    faceDetect(false),
    backGround(false),
    alpha(1),
    beta(0),
    numOfScreen(0)
{
    capture = new cv::VideoCapture();
    haarCascade = cv::CascadeClassifier("/home/darkness/Applications/OpenCV/data/haarcascades/haarcascade_frontalface_default.xml");
    pMOG = cv::bgsegm::createBackgroundSubtractorMOG();
}

WorkerClass::~WorkerClass()  {
    if (capture->isOpened()) capture->release();
    delete capture;

    if (writer.isOpened()) writer.release();
    //delete writer;
}


//ВСЕ RECIEVE-МЕТОДЫ...........................................................


void WorkerClass::receiveGrabFrame() {
    if (!toggleStream) return;

    /*(*capture) >> frameOriginal;
    if (frameOriginal.empty()) return;*/

    (*capture) >> frame;
    if (frame.empty()) return;
    //

    process();

    QImage output = mat2qimg(frameOriginal);

    if (backGround) {
        output = mat2qimg(frameHelper_show);
    }

    emit sendFrame(output);
}

void WorkerClass::receiveRecording() {
    if (!toggleStream2) return;

    recordVideo();
}

void WorkerClass::receiveSetup() {
    checkIfDeviceAlreadyOpened();
    if(capture->isOpened()) {
        status = false;
        return;
    }

    status = true;
}

void WorkerClass::receiveSetup2() {
    checkIfWriterOpened();

}

void WorkerClass::receivePrintScreen() {
    if (backGround) {
        cv::cvtColor(frameHelper_show, frameHelper_screen, CV_RGB2BGR);
    }
    else {
        cv::cvtColor(frameOriginal, frameHelper_screen, CV_RGB2BGR);
    }

    std::sprintf(fileName, "file%d.jpg", numOfScreen);
    cv::imwrite(fileName, frameHelper_screen);

    numOfScreen++;
}

// /////////////////////////////////////////////////////////////////////////////////

void WorkerClass::receiveToggleStream() {
    toggleStream = !toggleStream;
}

void WorkerClass::receiveToggleStream2() {
    toggleStream2 = !toggleStream2;
}

void WorkerClass::receiveFaceDetect() {
    faceDetect = !faceDetect;
}

void WorkerClass::receiveBackground() {
    backGround = !backGround;
}

void WorkerClass::receiveBrightness(int b) {
    beta = b;
}

void WorkerClass::receiveContrast(int a) {
    alpha = a;
}



//ВСЕ ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ...................................................



void WorkerClass::checkIfDeviceAlreadyOpened() {
    if(capture->isOpened()) capture->release();
    //capture->open(device);
    capture->open("/home/darkness/video2.avi");
}

void WorkerClass::checkIfWriterOpened() {
    if (writer.isOpened()) writer.release();
    writer.open("/home/darkness/file.avi", CV_FOURCC('X','V','I','D'), capture->get(CV_CAP_PROP_FPS), cv::Size(capture->get(CV_CAP_PROP_FRAME_WIDTH), capture->get(CV_CAP_PROP_FRAME_HEIGHT)), 1);
}

void WorkerClass::process() {
    //
    cv::cvtColor(frame, frameOriginal, CV_BGR2RGB);
    //
    //cv::cvtColor(frameOriginal, frameOriginal, CV_BGR2RGB);

    brightnessAndContrast();

    if (faceDetect) {
        detectAndDisplay(frameOriginal);
    }

    if (backGround) {
        deleteBackground();
    }
}

QImage WorkerClass::mat2qimg(cv::Mat image) {
    return QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);

}

void WorkerClass::recordVideo() {
    if (backGround) {
        cv::cvtColor(frameHelper_show, frameHelper_screen, CV_RGB2BGR);
        writer.write(frameHelper_screen);
    }
    else {
        cv::cvtColor(frameOriginal, frameHelper_screen, CV_RGB2BGR);
        writer.write(frameHelper_screen);
    }

}


//ВСЕ МЕТОДЫ ПО ОБРАБОТКЕ ВИДЕО.................................................................


void WorkerClass::detectAndDisplay(cv::Mat image) {
    cv::cvtColor(image, frameHelper_detect, CV_RGB2GRAY);

    cv::equalizeHist(frameHelper_detect, frameHelper_detect);
    haarCascade.detectMultiScale(frameHelper_detect, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30), cv::Size(50, 50));

    for( size_t i = 0; i < faces.size(); i++ )
      {
        cv::Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        cv::ellipse(image, center, cv::Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, cv::Scalar( 0, 139, 139 ), 4, 8, 0 );
      }
}

void WorkerClass::brightnessAndContrast() {

    for (int x = 0; x < frameOriginal.rows; x++) {
        for (int y = 0; y < frameOriginal.cols; y++) {
            for (int c = 0; c < 3; c++) {
                frameOriginal.at<cv::Vec3b>(x,y)[c] = cv::saturate_cast<uchar>(alpha * (frameOriginal.at<cv::Vec3b>(x,y)[c]) + beta);
            }
        }
    }
}

void WorkerClass::deleteBackground() {

    pMOG->apply(frameOriginal, frameHelper_bg);

    cv::cvtColor(frameHelper_bg, frameHelper_show, CV_GRAY2RGB);
}


