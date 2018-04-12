#include "imagechanging.h"
#include "ui_imagechanging.h"
#include <QFileDialog>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


ImageChanging::ImageChanging(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageChanging),
    erosion_size(0),
    dilation_size(0),
    max_kernel_size(21),
    alpha(1),
    beta(0),
    step(0),
    bw(false)
{
    ui->setupUi(this);
}

ImageChanging::~ImageChanging()
{
    delete ui;
}

void ImageChanging::on_loadImage_clicked()
{
    ui->loadImage->setDisabled(true);

    ui->brightness->setDisabled(false);
    ui->contrast->setDisabled(false);
    ui->sharpness->setDisabled(false);
    ui->erosion->setDisabled(false);
    ui->dilation->setDisabled(false);
    ui->bw->setDisabled(false);
    //ui->saveImage->setDisabled(false);
    ui->applyChanges->setDisabled(false);

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home/darkness",tr("Images (*.png *.jpg)"));

    if (fileName.isEmpty()) {
        return;
    }

    currentImage = cv::imread(fileName.toStdString());
    //original = cv::imread(fileName.toStdString());
    cvtColor(currentImage, currentImage, CV_BGR2RGB);
    //cvtColor(original, original, CV_BGR2RGB);
    currentImage.copyTo(original);

    ui->currentFrame->setScaledContents(true);

    ui->currentFrame->setPixmap(mat2pix(currentImage));

    connect(ui->erosion, SIGNAL(valueChanged(int)), this, SLOT(receiveErosion(int)));
    connect(ui->dilation, SIGNAL(valueChanged(int)), this, SLOT(receiveDilation(int)));

    connect(ui->brightness, SIGNAL(valueChanged(int)), this, SLOT(receiveBrightness(int)));
    connect(ui->contrast, SIGNAL(valueChanged(int)), this, SLOT(receiveContrast(int)));

    connect(ui->sharpness, SIGNAL(valueChanged(int)), this, SLOT(receiveSharpness(int)));

    connect(ui->bw, SIGNAL(clicked(bool)), this, SLOT(receiveBW(bool)));
}

void ImageChanging::on_applyChanges_clicked()
{
    /*if (!changedImage.empty()) {
        changedImage.copyTo(currentImage);*/
    if (!original.empty()) {
        original.copyTo(currentImage);

        ui->erosion->setValue(0);
        ui->dilation->setValue(0);
        ui->brightness->setValue(0);
        ui->contrast->setValue(1);
        ui->sharpness->setValue(0);

        ui->saveImage->setDisabled(false);
    }
    else return;
}

void ImageChanging::on_saveImage_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "/home/darkness/untitled.jpg",
                               tr("Images (*.png *.jpg)"));

    if (fileName.isEmpty()) return;

    if (bw == true) {
        cvtColor(currentImage, currentImage, CV_RGB2GRAY);
        cvtColor(currentImage, currentImage, CV_GRAY2BGR);
    }
    else {
        cvtColor(currentImage, currentImage, CV_RGB2BGR);
    }

    cv::imwrite( fileName.toStdString(), currentImage);

    ui->loadImage->setDisabled(false);

    ui->brightness->setDisabled(true);
    ui->contrast->setDisabled(true);
    ui->sharpness->setDisabled(true);
    ui->erosion->setDisabled(true);
    ui->dilation->setDisabled(true);
    ui->bw->setDisabled(true);
    ui->applyChanges->setDisabled(true);
}

///////////////////////////////////////////////////////////////////////

QPixmap ImageChanging::mat2pix(cv::Mat image) {
    return QPixmap::fromImage(QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888));
}


void ImageChanging::receiveErosion(int er) {
    erosion_size = er;
    erosion();
}

void ImageChanging::receiveDilation(int dil) {
    dilation_size = dil;
    dilation();
}

void ImageChanging::receiveBrightness(int b) {
    beta = b;
    brightnessAndContrast();
}

void ImageChanging::receiveContrast(int a) {
    alpha = a;
    brightnessAndContrast();
}

void ImageChanging::receiveSharpness(int k) {
    step = k;
    sharpness();
}

void ImageChanging::receiveBW(bool bwh) {
    bw = bwh;
    bAndW();
}

void ImageChanging::erosion() {
    int erosion_type = cv::MORPH_CROSS;
    cv::Mat element = cv::getStructuringElement(erosion_type, cv::Size(2*erosion_size + 1, 2*erosion_size + 1), cv::Point(erosion_size,erosion_size));

    cv::erode(currentImage,changedImage,element);

    cv::erode(currentImage, original, element);


    if (bw == true) {
        cv::cvtColor(changedImage, changedImage, CV_RGB2GRAY);
        ui->currentFrame->setPixmap(QPixmap::fromImage(mat2im(changedImage)));
    }
    else {
        ui->currentFrame->setPixmap(mat2pix(changedImage));

        //changedImage.copyTo(original);
    }

}

void ImageChanging::dilation() {
    int dilation_type = cv::MORPH_CROSS;
    cv::Mat element = cv::getStructuringElement(dilation_type, cv::Size(2*dilation_size + 1, 2*dilation_size + 1), cv::Point(dilation_size, dilation_size));

    cv::dilate(currentImage,changedImage,element);

    cv::dilate(currentImage, original, element);

    if (bw == true) {
        cv::cvtColor(changedImage, changedImage, CV_RGB2GRAY);
        ui->currentFrame->setPixmap(QPixmap::fromImage(mat2im(changedImage)));
    }
    else {
        ui->currentFrame->setPixmap(mat2pix(changedImage));
    }

    //changedImage.copyTo(original);

}

void ImageChanging::brightnessAndContrast() {
    changedImage = cv::Mat::zeros(currentImage.size(), currentImage.type());

    for (int x = 0; x < currentImage.rows; x++) {
        for (int y = 0; y < currentImage.cols; y++) {
            for (int c = 0; c < 3; c++) {
                changedImage.at<cv::Vec3b>(x,y)[c] = cv::saturate_cast<uchar>(alpha * (currentImage.at<cv::Vec3b>(x,y)[c]) + beta);

                original.at<cv::Vec3b>(x,y)[c] = cv::saturate_cast<uchar>(alpha * (currentImage.at<cv::Vec3b>(x,y)[c]) + beta);
            }
        }
    }


    if (bw == true) {
        cv::cvtColor(changedImage, changedImage, CV_RGB2GRAY);
        ui->currentFrame->setPixmap(QPixmap::fromImage(mat2im(changedImage)));
    }
    else {
        ui->currentFrame->setPixmap(mat2pix(changedImage));
    }

    //changedImage.copyTo(original);
}

void ImageChanging::sharpness() {
    if (step < 0) {
        cv::blur(currentImage, changedImage, cv::Size(-step * 2 + 1, -step * 2 + 1));
        cv::blur(currentImage, original, cv::Size(-step * 2 + 1, -step * 2 + 1));
    }
    else {
        //currentImage.copyTo(changedImage);
        float matr[9] {
                        -0.0375 - 0.05*step, -0.0375 - 0.05*step, -0.0375 - 0.05*step,
                        -0.0375 - 0.05*step, 1.3 + 0.4*step, -0.0375 - 0.05*step,
                        -0.0375 - 0.05*step, -0.0375 - 0.05*step, -0.0375 - 0.05*step
                    };
        cv::Mat kernel_matrix = cv::Mat(3, 3, CV_32FC1, &matr);
        cv::filter2D(currentImage, changedImage, 32, kernel_matrix);

        cv::filter2D(currentImage, original, 32, kernel_matrix);
    }

    if (bw == true) {
        cv::cvtColor(changedImage, changedImage, CV_RGB2GRAY);
        ui->currentFrame->setPixmap(QPixmap::fromImage(mat2im(changedImage)));
    }
    else {
        ui->currentFrame->setPixmap(mat2pix(changedImage));
    }

    //changedImage.copyTo(original);
}

void ImageChanging::bAndW() {
    changedImage = cv::Mat::zeros(currentImage.size(), currentImage.type());

    if (bw == true) {
        cvtColor(currentImage, changedImage, CV_RGB2GRAY);
        ui->currentFrame->setPixmap(QPixmap::fromImage(mat2im(changedImage)));
    }

    else {
        original.copyTo(currentImage);
        original.copyTo(changedImage);
        //cvtColor(currentImage, changedImage, CV_GRAY2RGB);
        ui->currentFrame->setPixmap(mat2pix(changedImage));
    }

}

QImage ImageChanging::mat2im(cv::Mat src) {
    int w=src.cols;
    int h=src.rows;
    QImage qim(w,h,QImage::Format_RGB32);
    QRgb pixel;
    cv::Mat im;
    normalize(src.clone(),im,0.0,255.0,CV_MINMAX,CV_8UC1);
    for(int i=0;i<w;i++)
    {
        for(int j=0;j<h;j++)
        {
            int gray = (int)im.at<unsigned char>(j, i);
            pixel = qRgb(gray,gray,gray);
            qim.setPixel(i,j,pixel);
        }
    }
    return qim;
}



