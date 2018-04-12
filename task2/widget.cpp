#include "widget.h"
#include "ui_widget.h"
#include "workerclass.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->currentFrame->setScaledContents(true);
    setup();
}

Widget::~Widget()
{
    thread->quit();
    while (!thread->isFinished());
    delete thread;

    delete ui;
}

void Widget::setup() {
    thread = new QThread();
    WorkerClass *worker = new WorkerClass();
    QTimer *workerTrigger = new QTimer();
    workerTrigger->setInterval(25);

    connect(workerTrigger, SIGNAL(timeout()), worker, SLOT(receiveGrabFrame()));
    connect(this, SIGNAL(sendSetup()), worker, SLOT(receiveSetup()));
    connect(this, SIGNAL(sendToggleStream()), worker, SLOT(receiveToggleStream()));

    connect(workerTrigger, SIGNAL(timeout()), worker, SLOT(receiveRecording()));
    connect(this, SIGNAL(sendToggleStream2()), worker, SLOT(receiveToggleStream2()));
    connect(ui->recordingVideo, SIGNAL(clicked(bool)), this, SLOT(receiveToggleStream2()));
    connect(this, SIGNAL(sendSetup2()), worker, SLOT(receiveSetup2()));

    connect(ui->startCapturing, SIGNAL(clicked(bool)), this, SLOT(receiveToggleStream()));
    connect(ui->faceDetection, SIGNAL(toggled(bool)), worker, SLOT(receiveFaceDetect()));
    connect(ui->backgroundSubstruct, SIGNAL(toggled(bool)), worker, SLOT(receiveBackground()));
    connect(ui->brightness, SIGNAL(valueChanged(int)), worker, SLOT(receiveBrightness(int)));
    connect(ui->contrast, SIGNAL(valueChanged(int)), worker, SLOT(receiveContrast(int)));
    connect(ui->screenShot, SIGNAL(clicked(bool)), worker, SLOT(receivePrintScreen()));
    connect(worker, SIGNAL(sendFrame(QImage)), this, SLOT(receiveFrame(QImage)));

    workerTrigger->start();
    worker->moveToThread(thread);
    workerTrigger->moveToThread(thread);

    thread->start();

    emit sendSetup();
    emit sendSetup2();
}

void Widget::receiveFrame(QImage frame) {

    ui->currentFrame->setPixmap(QPixmap::fromImage(frame));
}

void Widget::receiveToggleStream() {
    if(!ui->startCapturing->text().compare("Play")) {
        ui->startCapturing->setText("Pause");
        ui->faceDetection->setDisabled(false);
        ui->screenShot->setDisabled(false);
        ui->brightness->setDisabled(false);
        ui->contrast->setDisabled(false);
        ui->recordingVideo->setDisabled(false);
        ui->backgroundSubstruct->setDisabled(false);

    }
    else {
        ui->startCapturing->setText("Play");
        ui->faceDetection->setDisabled(true);
        ui->screenShot->setDisabled(true);
        ui->brightness->setDisabled(true);
        ui->contrast->setDisabled(true);
        ui->recordingVideo->setDisabled(true);
        ui->backgroundSubstruct->setDisabled(true);

    }

    emit sendToggleStream();
}

void Widget::receiveToggleStream2() {
    if (!ui->recordingVideo->text().compare("Record")) {
        ui->recordingVideo->setText("Stop");
    }
    else {
        ui->recordingVideo->setText("Record");
    }

    emit sendToggleStream2();
}
