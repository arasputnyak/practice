#include "cvmainc.h"

CvMainC::CvMainC(QObject *parent) :
    QObject(parent),
    //показ видео
    status(false),
    toggleStream(false),
    //обработка
    peopleCount(false),
    dayNight(false),
    borderCross(false),
    sizeOfArray(0),
    sizeOfArray_2(0),
    numOfPeople(0),
    numOfPeople_2(0),
    numOfVideofile(0),
    counter(0),
    cnt_size(0),
    //вспомогательное
    flagok_DN(false),
    flagok_BC(false),
    flagok_AO(false),
    flagok_AO2(false),
    fout("/home/darkness/CompVision/PCount/file.txt"),
    fout_2("/home/darkness/CompVision/TOfDay/file.txt"),
    fout_3("/home/darkness/CompVision/BCross/file.txt"),
    fout_4("/home/darkness/CompVision/AObjects/file.txt"),
    rectangles(10),
    countArray(10)
{
    capture = new VideoCapture();
    haarCascade = CascadeClassifier("/home/darkness/haarcascade_fullbody.xml");
    pMOG = bgsegm::createBackgroundSubtractorMOG();
    pMOG2 = bgsegm::createBackgroundSubtractorMOG();
    pointer=rectangles.data();

    if (fout.exists()) {
        fout.remove();
    }

    if (fout_2.exists()) {
        fout_2.remove();
    }

    if (fout_3.exists()) {
        fout_3.remove();
    }

    if (fout_4.exists()) {
        fout_4.remove();
    }
}

CvMainC::~CvMainC() {
    if (capture->isOpened()) capture->release();
    delete capture;

    if (writer.isOpened()) writer.release();

}

//ВСЕ RECIEVE-МЕТОДЫ...........................................................

void CvMainC::receiveGrabFrame() {
    if (!toggleStream) return;

    (*capture) >> frame;
    if (frame.empty()) return;

    process();

    QImage output = mat2qimg(frame_original);

    emit sendFrame(output);
}

void CvMainC::receiveSetup() {
    checkIfDeviceAlreadyOpened();
    if(capture->isOpened()) {
        status = false;
        return;
    }
    status = true;
}

void CvMainC::receiveToggleStream() {
    toggleStream = !toggleStream;
}

void CvMainC::receiveCountingPeople() {
    peopleCount = !peopleCount;
}

void CvMainC::receiveTimeOfDay() {
    dayNight = !dayNight;
}

void CvMainC::receiveBorderCrossing() {
    borderCross = !borderCross;
}

void CvMainC::receiveAbandonedObjects() {
    abObjects = !abObjects;
}

// ОСНОВНОЕ..................................................

void CvMainC::checkIfDeviceAlreadyOpened() {
    if(capture->isOpened()) capture->release();

    //capture->open("/home/darkness/video3.avi");    // подсчет людей + пересечение границы
    capture->open("/home/darkness/video.avi");    // время суток
    //capture->open("/home/darkness/test1.avi");    // оставленный предмет

}

void CvMainC::process() {

    cvtColor(frame, frame_original, CV_BGR2RGB);

    if (frame_background_long.empty()) {
        frame_original.copyTo(frame_background_long);
        frame_original.copyTo(frame_background_short);
        frame_object = Mat::zeros(frame_background_long.rows, frame_background_long.cols, CV_8UC1);
    }

    time_t seconds = time(NULL);
    tm* timeinfo = localtime(&seconds);
    cur_time = timeinfo->tm_hour;
    fps = (int) capture->get(CV_CAP_PROP_FPS);

    if (peopleCount) {
        detectAndDisplay(frame_original, frame_original, &sizeOfArray, 0);
        if (numOfPeople != sizeOfArray && sizeOfArray != 0) {
            numOfPeople = sizeOfArray;
            if (fout.open(QIODevice::WriteOnly | QIODevice::Append)) {
                QTextStream out(&fout);
                out.setCodec("UTF-8");
                out<<"people: "<<numOfPeople<<" time: "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                fout.close();
            }
        }
    }

    if (dayNight) {
        cvtColor(frame, frame_dayNight, CV_BGR2GRAY);
        threshold(frame_dayNight, frame_dayNight, 100, 255, 0);

        t = mean(frame_dayNight);
        avg_int = t[0];
        if ((((cur_time >= 22) || (cur_time >= 0 && cur_time <=7)) && avg_int > 40) || ((cur_time > 7 && cur_time < 22) && avg_int < 40)) {
            if (flagok_DN == false) {
                flagok_DN = true;
                if (avg_int > 40) {
                    if (fout_2.open(QIODevice::WriteOnly | QIODevice::Append)) {
                        QTextStream out_2(&fout_2);
                        out_2.setCodec("UTF-8");
                        out_2<<"smth is happening during the night: "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                        fout_2.close();
                    }
                } else {
                    if (fout_2.open(QIODevice::WriteOnly | QIODevice::Append)) {
                        QTextStream out_2(&fout_2);
                        out_2.setCodec("UTF-8");
                        out_2<<"smth is happening during the day: "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                        fout_2.close();
                    }
                }
            }
            if (flagok_DN == true) {
                if (avg_int < 40) {
                    if (avg_int > 10){
                        brightnessAndContrast(frame_original, int(avg_int/2));
                    } else {
                        /*
                        if (lights.status == 0) {
                            turn(lights);
                            lights.status = 1;
                        }
                        */
                    }
                }
            }
        } else {
            if (flagok_DN == true) {
                flagok_DN = false;
                if (avg_int < 40) {
                    if (fout_2.open(QIODevice::WriteOnly | QIODevice::Append)) {
                        QTextStream out_2(&fout_2);
                        out_2.setCodec("UTF-8");
                        out_2<<"evrthg is fine now (night): "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                        fout_2.close();
                    }
                } else {
                    if (fout_2.open(QIODevice::WriteOnly | QIODevice::Append)) {
                        QTextStream out_2(&fout_2);
                        out_2.setCodec("UTF-8");
                        out_2<<"evrthg is fine now (day): "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                        fout_2.close();
                    }
                }
            }
        }
    }

    if (borderCross) {
        pMOG->apply(frame_original, frame_MOG);

        Rect rect(0, 160, 480, 180);
        rectangle(frame_original, rect, Scalar(139, 34, 82), 4, 8, 0);
        frame_ROI = frame_MOG(rect);
        frame_ROI2 = frame_original(rect);
        t_2 = mean(frame_ROI);
        avg_int_2 = t_2[0];

        if (avg_int_2 > 4) {
            if (flagok_BC == false) {
                flagok_BC = true;
                if (fout_3.open(QIODevice::WriteOnly | QIODevice::Append)) {
                    QTextStream out_3(&fout_3);
                    out_3.setCodec("UTF-8");
                    out_3<<"someone has crossed the line: "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                    fout_3.close();
                }

                sprintf(fileName, "/home/darkness/file%d.avi", numOfVideofile);
                if (writer.isOpened()) {
                    writer.release();
                }
                writer.open(fileName, CV_FOURCC('X','V','I','D'), capture->get(CV_CAP_PROP_FPS), cv::Size(capture->get(CV_CAP_PROP_FRAME_WIDTH), capture->get(CV_CAP_PROP_FRAME_HEIGHT)), 1);
                numOfVideofile++;
            }
            detectAndDisplay(frame_ROI2, frame_original, &sizeOfArray_2, 160);
            writer.write(frame_original);
            if (numOfPeople_2 != sizeOfArray_2 && sizeOfArray_2 != 0) {
                numOfPeople_2 = sizeOfArray_2;
                if (fout_3.open(QIODevice::WriteOnly | QIODevice::Append)) {
                    QTextStream out_3(&fout_3);
                    out_3.setCodec("UTF-8");
                    out_3<<"    people in the region: "<<numOfPeople_2<<" time: "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                    fout_3.close();
                }
            }
        } else {
            if (flagok_BC == true) {
                flagok_BC = false;
                if (fout_3.open(QIODevice::WriteOnly | QIODevice::Append)) {
                    QTextStream out_3(&fout_3);
                    out_3.setCodec("UTF-8");
                    out_3<<"everyone has left the region, time: "<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                    fout_3.close();
                }
                writer.release();
            }
        }
    }

    if (abObjects) {

        for (unsigned int i = 0; i < rectangles.size(); i++) {
            if (rectangles[i].size().area() != 0 && countArray[i] < fps + 10) {
                countArray[i]++;
            }
        }

        for (unsigned int i = 0; i < rectangles.size(); i++) {
            if (countArray[i] < fps) {
                drawing(frame_original, i);
            }
        }

        pMOG2->apply(frame_original, frame_MOG2);

        t_3 = mean(frame_MOG2);
        avg_int_3 = t_3[0];

        if (avg_int_3 > 0.2 || flagok_AO2 == true) {

            for (int x = 0; x < frame_original.rows; x++) {
                for (int y = 0; y < frame_original.cols; y++) {
                    if (frame_MOG2.at<uchar>(x, y) == 0) {
                        if ((frame_original.at<Vec3b>(x, y)[0] - frame_background_short.at<Vec3b>(x, y)[0] < 10) &&
                            (frame_original.at<Vec3b>(x, y)[1] - frame_background_short.at<Vec3b>(x, y)[1] < 10) &&
                            (frame_original.at<Vec3b>(x, y)[2] - frame_background_short.at<Vec3b>(x, y)[2] < 10)) {
                            frame_object.at<uchar>(x, y) = 0;
                        } else {
                            frame_object.at<uchar>(x, y) = 255;
                        }
                    }
                }
            }

            resize(frame_object, frame_object, Size(8, 8), 0, 0, INTER_AREA);
            resize(frame_object, frame_object, Size(frame_original.cols, frame_original.rows), 0, 0, INTER_LINEAR);

            threshold(frame_object, frame_object, 100, 255, THRESH_BINARY);

            t_h = mean(frame_object);
            avg_int_h = t_h[0];

            if (avg_int_h > 1) {
                if (flagok_AO2 == false) {
                    flagok_AO2 = true;
                }

                frame_original.copyTo(frame_help);

                if (counter == 0) {
                    flagok_AO = true;
                }

                counter += 1;

                if (counter / fps >= 2) {

                    fndCont(frame_object);

                    if (flagok_AO == true) {
                        if (cnt_size != 0) {
                            if (fout_4.open(QIODevice::WriteOnly | QIODevice::Append)) {
                                QTextStream out_4(&fout_4);
                                out_4.setCodec("UTF-8");
                                out_4<<"objects left: "<<cnt_size<<" time:"<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                                fout_4.close();
                            }
                        } else {
                            if (fout_4.open(QIODevice::WriteOnly | QIODevice::Append)) {
                                QTextStream out_4(&fout_4);
                                out_4.setCodec("UTF-8");
                                out_4<<"the object was taken away, time:"<<QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy")<<"\n";
                                fout_4.close();
                            }
                        }

                        for (int x = 0; x < frame_background_long.rows; x++) {
                            for (int y = 0; y < frame_background_long.cols; y++) {
                                if (frame_MOG2.at<uchar>(x, y) == 255) {
                                    frame_background_short.at<Vec3b>(x, y)[0] = frame_background_long.at<Vec3b>(x, y)[0];
                                    frame_background_short.at<Vec3b>(x, y)[1] = frame_background_long.at<Vec3b>(x, y)[1];
                                    frame_background_short.at<Vec3b>(x, y)[2] = frame_background_long.at<Vec3b>(x, y)[2];
                                } else {
                                    frame_background_short.at<Vec3b>(x, y)[0] = frame_help.at<Vec3b>(x, y)[0];
                                    frame_background_short.at<Vec3b>(x, y)[1] = frame_help.at<Vec3b>(x, y)[1];
                                    frame_background_short.at<Vec3b>(x, y)[2] = frame_help.at<Vec3b>(x, y)[2];
                                }
                            }
                        }
                        flagok_AO = false;
                        flagok_AO2 = false;
                        counter = 0;
                    }
                }
            } else {
                if (flagok_AO2 == true) {
                    flagok_AO2 = false;
                }
                if (flagok_AO == true) {
                    flagok_AO = false;
                    counter = 0;
                }
            }
        }
    }

}


QImage CvMainC::mat2qimg(Mat image) {
    return QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);

}

//ВСЕ МЕТОДЫ ПО ОБРАБОТКЕ ВИДЕО.................................................................

void CvMainC::detectAndDisplay(Mat frame1, Mat frame2, size_t* size, int coord) {
    vector<Rect> people;
    Mat frame_gray;
    cvtColor(frame1, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    haarCascade.detectMultiScale(frame_gray, people, 1.05, 4, 0 | CASCADE_SCALE_IMAGE, Size(20, 10), Size(160, 120));

    (*size) = people.size();

    for (size_t i = 0; i < people.size(); i++) {
        if (coord == 0) {
            rectangle(frame2, people[i].tl() + Point(0, coord), people[i].br() + Point(0, coord), Scalar(50, 205, 50), 2, 8, 0);
        } else {
            rectangle(frame2, people[i].tl() + Point(-10, coord-30), people[i].br() + Point(10, coord+20), Scalar(139, 0, 0), 2, 8, 0);
        }
    }
}

void CvMainC::drawing(Mat frame, int i) {
    rectangle(frame, rectangles[i].tl(), rectangles[i].br(), Scalar(255, 0, 0), 2, 8, 0 );
}

void CvMainC::fndCont(Mat frame) {
    int position = 0;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect(contours.size());

    for (unsigned int i = 0; i < rectangles.size(); i++) {
            if (rectangles[i].size().area() == 0) {
                position = i;
                break;
            }
        }

    for(unsigned int i = 0; i < contours.size(); i++ )
    { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );

        pointer[i + position]  = boundingRect( Mat(contours_poly[i]) );
    }

    cnt_size = contours.size();
}

void CvMainC::brightnessAndContrast(Mat frame, int p) {

    for (int x = 0; x < frame.rows; x++) {
        for (int y = 0; y < frame.cols; y++) {
            for (int c = 0; c < 3; c++) {
                frame.at<Vec3b>(x,y)[c] = saturate_cast<uchar>(2 * (frame.at<Vec3b>(x,y)[c]) + p);
            }
        }
    }
}
