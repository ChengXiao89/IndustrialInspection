#include "client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QThread>
#include <QUuid>

fiber_end_client::fiber_end_client(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::connected, this, &fiber_end_client::onConnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &fiber_end_client::onReadyRead);
}

void fiber_end_client::set_server_config(const QString& ip, quint16 port)
{
    m_server_ip = ip;
    m_server_port = port;
}

bool fiber_end_client::is_server_running()
{
    QTcpSocket testSocket;
    testSocket.connectToHost(m_server_ip, m_server_port);
    bool connected = testSocket.waitForConnected(200);
    testSocket.disconnectFromHost();
    return connected;
}

void fiber_end_client::start_server()
{
#ifdef Q_OS_WIN     //windows操作系统
    QProcess::startDetached("fiber_end_server.exe", { m_server_ip, QString::number(m_server_port) });
#else               //其他操作系统
    QProcess::startDetached("./fiber_end_server", { m_server_ip, QString::number(m_server_port) });
#endif
}

void fiber_end_client::start()
{
    if (!is_server_running())
    {
        qDebug() << QString::fromStdString("后端未运行，启动后端...");
        start_server();
    }
    // 设置定时器，每 300ms 尝试连接一次
    connect(&m_retry_timer, &QTimer::timeout, this, &fiber_end_client::on_try_connect);
    m_retry_count = 0;
    m_retry_timer.start(300);
}

void fiber_end_client::on_try_connect()
{
    if (m_retry_count >= m_max_retry) 
    {
        m_retry_timer.stop();
        qDebug() << QString::fromStdString("连接后端超时, 启动失败或后端无响应");
        return;
    }
    m_retry_count++;
    // 尝试连接. 这里用一个临时 socket 测试是否能连接成功
    QTcpSocket testSocket;
    testSocket.connectToHost(m_server_ip, m_server_port);
    bool connected = testSocket.waitForConnected(100); // 等待 100ms
    testSocket.disconnectFromHost();
    if (connected) 
    {
        m_retry_timer.stop();
        qDebug() << QString::fromStdString("后端已启动, 连接成功!");
        // 连接主 socket
        m_socket.connectToHost(m_server_ip, m_server_port);
    }
}


void fiber_end_client::onConnected()
{
    qDebug() << QString::fromStdString("已连接到后端 ") << m_server_ip << ":" << m_server_port;
    QJsonObject obj;
    obj["command"] = "client_request_server_parameter";        
    obj["param"] = 0;
	obj["request_id"] = generateUniqueRequestId(); // 生成唯一请求 ID
    m_socket.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_socket.flush();
}

void fiber_end_client::onReadyRead()
{
    QByteArray data = m_socket.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    qDebug() << "收到后端消息: " << obj["command"].toString() << obj["param"].toInt();
    receive_message(obj);
}

void fiber_end_client::send_message(const QJsonObject& obj)
{
    m_socket.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_socket.flush();
}

void fiber_end_client::receive_message(const QJsonObject& obj)
{
    if (obj["command"].toString() == QString("server_camera_parameter"))
    {
        emit post_update_camera_status(QVariant::fromValue(obj));
    }
	if(obj["command"].toString() == QString("server_camera_list"))
	{
        emit post_update_camera_list(QVariant::fromValue(obj));
	}
    else if (obj["command"].toString() == QString("server_camera_opened_success"))
    {
        emit post_camera_opened_success(QVariant::fromValue(obj));
    }
    else if (obj["command"].toString() == QString("server_camera_closed_success"))
    {
        emit post_camera_closed_success();
    }
    else if(obj["command"].toString() == QString("server_camera_parameter_changed_success"))
    {
        emit post_camera_parameter_changed_success(QVariant::fromValue(obj));
	}
    else if (obj["command"].toString() == QString("server_camera_grab_set_success"))
    {
        emit post_camera_grab_set_success(QVariant::fromValue(obj));
    }
	else if(obj["command"].toString() == QString("server_camera_trigger_once_success"))
    {
        emit post_camera_trigger_once_success(QVariant::fromValue(obj));
    }
    if(obj["command"].toString() == QString("server_report_error"))
    {
        QString error_message = obj["param"].toString();
        emit post_server_error(error_message);
	}
}

QString fiber_end_client::generateUniqueRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}