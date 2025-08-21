#include "IndustrialInspection.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    IndustrialInspection window;
    window.show();
    return app.exec();
}
