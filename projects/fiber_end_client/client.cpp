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
    QDataStream in(&m_socket);
    in.setByteOrder(QDataStream::BigEndian);

    const qint32 MAX_MESSAGE_SIZE = 1024 * 1024; // 最大1MB
    const int MAX_MESSAGES_PER_CALL = 10; // 防止阻塞
    int processedCount = 0;

    while (processedCount < MAX_MESSAGES_PER_CALL) 
    {
        // 如果还没有读到长度前缀
        if (m_block_size == 0) 
        {
            if (m_socket.bytesAvailable() < (int)sizeof(qint32))
                return;
            in >> m_block_size;
            if (m_block_size <= 0 || m_block_size > MAX_MESSAGE_SIZE)   //阻止内存攻击
            {
                qWarning() << "收到非法消息长度:" << m_block_size;
                m_block_size = 0;  // 重置准备下一条
                return;
            }
        }

        // 检查是否到齐
        if (m_socket.bytesAvailable() < m_block_size)
            return;

        QByteArray payload = m_socket.read(m_block_size);
        if (payload.size() != m_block_size) 
        {
            qWarning() << "读取消息不完整，期望:" << m_block_size << "实际:" << payload.size();
            m_block_size = 0;
            return;
        }
        m_block_size = 0;
        processedCount++;
        // JSON 解析
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError) 
        {
            qWarning() << "JSON 解析失败:" << parseError.errorString() << "前100字节 HEX:" << payload.left(100).toHex();
            m_block_size = 0;
            return;
        }

        QJsonObject obj = doc.object();
        //qDebug() << "收到消息:" << obj;
        receive_message(obj);
    }
    // 如果还有数据，延迟继续处理
    if (m_socket.bytesAvailable() > 0) 
    {
        QTimer::singleShot(0, this, &fiber_end_client::onReadyRead);
    }
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
    else if (obj["command"].toString() == QString("server_set_motion_parameter_success"))
    {
        emit post_motion_parameter_changed_success(QVariant::fromValue(obj));
    }
    else if (obj["command"].toString() == QString("server_move_camera_success"))
    {
        emit post_move_camera_success(QVariant::fromValue(obj));
    }
    else if (obj["command"].toString() == QString("server_anomaly_detection_once"))
    {
        emit post_anomaly_detection_once(QVariant::fromValue(obj));
    }

    else if (obj["command"].toString() == QString("server_anomaly_detection_finish"))
    {
        emit post_anomaly_detection_finish(QVariant::fromValue(obj));
    }
    else if (obj["command"].toString() == QString("server_calibration_success"))
    {
        emit post_calibration_success();
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