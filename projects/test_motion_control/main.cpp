#include <QtCore/QCoreApplication>

#include "motion_control.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::string port = "COM1";  // 串口号
    unsigned int baud_rate = 115200;

    motion_control motor(port, baud_rate);
    if (motor.is_opened())
    {
        //motor.move_distance(1, -1000, 1000);

        //motor.move_position(1, 0, 1000);
        motor.reset();

        motor.close();
    }


    
}