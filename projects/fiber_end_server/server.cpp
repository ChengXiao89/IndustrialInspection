﻿#include "server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

fiber_end_server::fiber_end_server(QString ip, quint16 port, QObject* parent)
	: QTcpServer(parent),m_server_ip(ip),m_server_port(port)
{
	m_thread_algorithm = new thread_algorithm(QString::fromStdString("算法处理子线程"), this);
    connect(m_thread_algorithm, &thread_base::post_task_finished, this, &fiber_end_server::on_algorithm_task_finished);
	m_thread_device_enum = new thread_device_enum(QString::fromStdString("设备枚举子线程"),this);
    m_thread_device_enum->set_device_manager(&m_device_manager);
    connect(m_thread_device_enum, &thread_base::post_task_finished, this, &fiber_end_server::on_device_enum_task_finished);
	m_thread_misc = new thread_misc(QString::fromStdString("其他任务子线程"), this);
	m_thread_misc->set_device_manager(&m_device_manager);
    connect(m_thread_misc, &thread_base::post_task_finished, this, &fiber_end_server::on_misc_task_finished);
    m_thread_motion_control = new thread_motion_control(QString::fromStdString("运动控制子线程"), this);
    connect(m_thread_motion_control, &thread_base::post_task_finished, this, &fiber_end_server::on_motion_control_task_finished);
}

void fiber_end_server::set_server_config(const QString& ip, quint16 port)
{
    m_server_ip = ip;
    m_server_port = port;
}

bool fiber_end_server::start()
{
    if (!listen(QHostAddress(m_server_ip), m_server_port))
    {
        qWarning() << QString::fromStdString("后端启动失败:") << errorString();
        return false;
    }
    //服务器启动时，首先 1.加载配置文件 2.启动运控模块 3.设置光源亮度
    load_config_file("./config.xml");         //加载配置文件，如果没有则使用默认值
    //启动运控模块
    if(!m_thread_misc->initialize(&m_config_data))
    {
        qWarning() << QString::fromStdString("启动运控模块失败:") << errorString();
        return false;
    }
    
    qDebug() << QString::fromStdString("后端已启动，监听端口:") << m_server_port;
	m_thread_algorithm->start();
    m_thread_motion_control->start();
    m_thread_device_enum->start();
    m_thread_misc->start();

    //启动线程之后
    return true;
}

bool fiber_end_server::load_config_file(const std::string& config_file_path)
{
    return m_config_data.load_from_file(config_file_path);
}


void fiber_end_server::stop()
{
    for (QTcpSocket* client : m_clients)
    {
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
    m_map_request_id_to_socket.clear();
    m_thread_algorithm->stop();
    m_thread_motion_control->stop();
    m_thread_device_enum->stop();
    m_thread_misc->stop();
}

void fiber_end_server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);
    connect(client, &QTcpSocket::readyRead, this, &fiber_end_server::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &fiber_end_server::onDisconnected);
    m_clients << client;
    qDebug() << QString::fromStdString("新前端已连接");
}

void fiber_end_server::onReadyRead()
{
    auto* client = qobject_cast<QTcpSocket*>(sender());
    QByteArray data = client->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    QString request_id = obj["request_id"].toString();
	m_map_request_id_to_socket[request_id] = client; // 保存请求 ID 和对应的客户端
    process_request(obj);
}

void fiber_end_server::onDisconnected()
{
    auto* client = qobject_cast<QTcpSocket*>(sender());
    for (QMap<QString, QTcpSocket*>::iterator iter = m_map_request_id_to_socket.begin();
        iter != m_map_request_id_to_socket.end(); )
    {
	    if (iter.value() == client)
	    {
            iter = m_map_request_id_to_socket.erase(iter);
	    }
	    else
	    {
            ++iter;
	    }
    }
    m_clients.removeAll(client);
    client->deleteLater();
}

void fiber_end_server::process_request(const QJsonObject& obj)
{
    QString command = obj["command"].toString();
    qDebug() << QString::fromStdString("收到消息:") << command;
    if (command == "client_request_server_parameter")
    {
        m_thread_device_enum->add_task(obj);
    }
    if (command == "client_request_camera_list")
    {
        m_thread_device_enum->add_task(obj);
    }
    else if(command == "client_request_open_camera")
    {
        m_thread_misc->add_task(obj);
    }
    else if (command == "client_request_close_camera")
    {
        m_thread_misc->add_task(obj);
    }
    else if(command == "client_request_change_camera_parameter")
    {
        m_thread_misc->add_task(obj);
    }
    else if(command == "client_request_start_grab")
    {
        m_thread_misc->add_task(obj);
    }
    else if (command == "client_request_trigger_once")
    {
        if(m_is_triggering.load())
        {
			return; // 如果正在采图，则忽略触发请求
        }
        m_is_triggering.store(true);
        m_thread_misc->add_task(obj);
    }
    else if(command == "client_request_move_camera")
    {
        if (m_is_triggering.load())
        {
            return; // 如果正在采图，则忽略触发请求
        }
        m_is_triggering.store(true);
        m_thread_misc->add_task(obj);
    }
    else if(command == "client_request_set_motion_parameter")
    {
        m_thread_misc->add_task(obj);
    }
    else if (command == "client_request_auto_focus")
    {
        m_thread_misc->add_task(obj);
    }
    else if (command == "client_request_calibration")
    {
        m_thread_misc->add_task(obj);
    }
    else if(command == "client_request_update_server_parameter")
    {
        m_thread_misc->add_task(obj);
    }
}

void fiber_end_server::send_process_result(const QVariant& result_data)
{
    QJsonObject obj = result_data.toJsonObject();
    auto iter = m_map_request_id_to_socket.find(obj["request_id"].toString());
    if (iter != m_map_request_id_to_socket.end())
    {
        QTcpSocket* client = iter.value();
        m_map_request_id_to_socket.erase(iter);
        if (client != nullptr)
        {
            client->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
            client->flush();
        }
    }
}

int fiber_end_server::get_task_type(const QJsonObject& obj)
{
    if (obj.contains(QString("task_type")))
    {
        return obj["task_type"].toInt();
    }
    return TASK_TYPE_ANY; //默认返回任意任务类型
}

QJsonObject fiber_end_server::server_parameter_to_json(const st_config_data& config_data)
{
    return config_data.save_to_json();
}

void fiber_end_server::on_algorithm_task_finished(const QVariant& task_data)
{
	
}

void fiber_end_server::on_device_enum_task_finished(const QVariant& task_data)
{
	QJsonObject obj = task_data.toJsonObject();
    /*********************************
	 * 枚举线程在执行 client_request_server_parameter 任务的时候返回的结果为
	 * obj["command"] = "client_request_server_parameter";
	 * obj["device_list"] = devices_array;  //设备列表
	 * obj["request_id"]    //原始请求 ID，不需要改变
	 * 这里再追加一个["camera"]和["parameter"]字段，将打开的相机信息和服务器的端面检测参数传递给前端
     *********************************/
    if(obj["command"].toString() == QString("client_request_server_parameter"))
    {
        // 修改命令为 server_camera_parameter. 客户端接收到该消息后，需要:
		// （1）初始化相机列表
		// （2）更新相机视图图标
		// （3）打开控制面板，显示参数以及更新控件状态
        //  (4) 更新控制面板的端面检测界面参数
		obj["command"] = "server_camera_parameter";
	    if(m_thread_misc->camera() != nullptr && m_thread_misc->camera()->m_is_opened)
        {
            QJsonObject camera_obj = thread_misc::camera_parameter_to_json(m_thread_misc->camera());
            obj["camera"] = camera_obj; // 将相机参数添加到返回结果中
        }
        else
        {
            obj["camera"] = QJsonObject(); // 如果没有打开相机，则返回空对象
        }
        obj["parameter"] = m_config_data.save_to_json(); // 将端面检测参数添加到返回结果中
		send_process_result(QVariant::fromValue(obj));           // 将任务结果发送给客户端
    }
    else
    {
        send_process_result(task_data);                 // 将任务结果发送给客户端
    }
	
}

void fiber_end_server::on_misc_task_finished(const QVariant& task_data)
{
    QJsonObject obj = task_data.toJsonObject();
    if (obj["command"].toString() == QString("server_camera_opened_success"))
    {
        obj["parameter"] = m_config_data.save_to_json(); // 将端面检测参数添加到返回结果中
        send_process_result(QVariant::fromValue(obj));
    }
    else
    {
        send_process_result(task_data);                 // 将任务结果发送给客户端
    }
    /**********************如果是采图任务，需要重置状态***************************/
    if(get_task_type(obj) == TASK_TYPE_TRIGGER_ONCE || get_task_type(obj) == TASK_TYPE_MOVE_CAMERA)
    {
        m_is_triggering.store(false); // 触发采图完成后，重置采图状态
    }
}

void fiber_end_server::on_motion_control_task_finished(const QVariant& task_data)
{
	
}