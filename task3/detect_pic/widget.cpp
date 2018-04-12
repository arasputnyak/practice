#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    numOfImg(1)
{
    ui->setupUi(this);
    ui->label->setScaledContents(true);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    if (numOfImg < 6) {
        std::sprintf(fileName, "file%d.jpg", numOfImg);

        frameOriginal = cv::imread(fileName);

        haarCasc = cv::CascadeClassifier("/home/darkness/cascade.xml");

        if(haarCasc.empty()) return;
        if (frameOriginal.empty()) return;

        cv::cvtColor(frameOriginal, frameOriginal, CV_BGR2RGB);

        detectAndDisplay(frameOriginal);

        ui->label->setPixmap(QPixmap::fromImage(mat2qimg(frameOriginal)));

        numOfImg++;
    }

    else return;
}

void Widget::detectAndDisplay(cv::Mat frame) {

    cv::cvtColor(frame, frameProcessed, CV_RGB2GRAY);

    cv::equalizeHist(frameProcessed, frameProcessed);
    haarCasc.detectMultiScale(frameProcessed, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, cv::Size(300,300));

    for( size_t i = 0; i < faces.size(); i++ )
      {
        cv::Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        cv::ellipse(frame, center, cv::Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, cv::Scalar( 255, 0, 255 ), 4, 8, 0 );

      }
}

QImage Widget::mat2qimg(cv::Mat image) {
    return QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
}
