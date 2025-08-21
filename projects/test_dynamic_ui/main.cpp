#include <QApplication>
#include "dynamic_builder.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DynamicFormBuilder* builder = new DynamicFormBuilder();
    builder->loadFromXmlFile("./test.xml");
    builder->show();

    return app.exec();
}
