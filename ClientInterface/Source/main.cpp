#include "myWidget.h"
#include <QApplication>

using namespace Core;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    myWidget w;
    w.show();

    return a.exec();
}