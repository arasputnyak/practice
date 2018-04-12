#include "widget.h"
#include "ui_widget.h"
#include "worker.h"
#include <QTimer>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->label->setScaledContents(true);
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
    Worker *worker = new Worker();
    QTimer *workerTrigger = new QTimer();
    workerTrigger->setInterval(40);

    ui->label->setScaledContents(true);

    connect(workerTrigger, SIGNAL(timeout()), worker, SLOT(receiveGrabFrame()));
    connect(this, SIGNAL(sendSetup()), worker, SLOT(receiveSetup()));
    connect(this, SIGNAL(sendToggleStream()), worker, SLOT(receiveToggleStream()));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(receiveToggleStream()));
    connect(ui->checkBox, SIGNAL(toggled(bool)), worker, SLOT(receiveHorseDetect()));
    connect(worker, SIGNAL(sendFrame(QImage)), this, SLOT(receiveFrame(QImage)));

    workerTrigger->start();
    worker->moveToThread(thread);
    workerTrigger->moveToThread(thread);

    thread->start();

    emit sendSetup();
}

void Widget::receiveFrame(QImage frame) {

    ui->label->setPixmap(QPixmap::fromImage(frame));
}

void Widget::receiveToggleStream() {
    if(!ui->pushButton->text().compare(">")) {
        ui->pushButton->setText("||");
        ui->checkBox->setDisabled(false);
    }
    else {
        ui->pushButton->setText(">");
        ui->checkBox->setDisabled(true);
    }

    emit sendToggleStream();
}

