#include <QtCore/QCoreApplication>
#include "server.h"
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QString ip = "127.0.0.1";
    quint16 port = 5555;

    // 命令行参数: backend.exe 192.168.1.100 6666
    if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = QString(argv[2]).toUShort();

    fiber_end_server server(ip,port);
    
    if(!server.start())
    {
		return -1;
    }
    return app.exec();
}