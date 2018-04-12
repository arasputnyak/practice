#include "imagechanging.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageChanging w;
    w.show();

    return a.exec();
}
