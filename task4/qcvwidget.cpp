#include "qcvwidget.h"
#include "ui_qcvwidget.h"
#include "cvmainc.h"

QCvWidget::QCvWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QCvWidget)
{
    ui->setupUi(this);
    ui->label->setScaledContents(true);
    setup();
}

QCvWidget::~QCvWidget()
{
    thread->quit();
    while (!thread->isFinished());
    delete thread;

    delete ui;
}

void QCvWidget::setup() {
    thread = new QThread();
    CvMainC *mainClass = new CvMainC();
    QTimer *mainCTrigger = new QTimer();
    mainCTrigger->setInterval(70);

    connect(mainCTrigger, SIGNAL(timeout()), mainClass, SLOT(receiveGrabFrame()));
    connect(this, SIGNAL(sendSetup()), mainClass, SLOT(receiveSetup()));
    connect(this, SIGNAL(sendToggleStream()), mainClass, SLOT(receiveToggleStream()));

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(receiveToggleStream()));
    connect(ui->checkBox, SIGNAL(toggled(bool)), mainClass, SLOT(receiveTimeOfDay()));
    connect(ui->checkBox_2, SIGNAL(toggled(bool)), mainClass, SLOT(receiveCountingPeople()));
    connect(ui->checkBox_3, SIGNAL(toggled(bool)), mainClass, SLOT(receiveBorderCrossing()));
    connect(ui->checkBox_4, SIGNAL(toggled(bool)), mainClass, SLOT(receiveAbandonedObjects()));
    connect(mainClass, SIGNAL(sendFrame(QImage)), this, SLOT(receiveFrame(QImage)));

    mainCTrigger->start();
    mainClass->moveToThread(thread);
    mainCTrigger->moveToThread(thread);

    thread->start();

    emit sendSetup();
}

void QCvWidget::receiveFrame(QImage frame) {

    ui->label->setPixmap(QPixmap::fromImage(frame));
}

void QCvWidget::receiveToggleStream() {
    if(!ui->pushButton->text().compare(">")) {
        ui->pushButton->setText("||");
        ui->checkBox->setDisabled(false);
        ui->checkBox_2->setDisabled(false);
        ui->checkBox_3->setDisabled(false);
        ui->checkBox_4->setDisabled(false);
    }
    else {
        ui->pushButton->setText(">");
        ui->checkBox->setDisabled(true);
        ui->checkBox_2->setDisabled(true);
        ui->checkBox_3->setDisabled(true);
        ui->checkBox_4->setDisabled(true);
    }

    emit sendToggleStream();
}
