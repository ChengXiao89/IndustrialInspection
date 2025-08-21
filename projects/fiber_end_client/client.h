#pragma once
#include <QTcpSocket>
#include <QProcess>
#include <QTimer>

class fiber_end_client : public QObject
{
    Q_OBJECT
public:
    explicit fiber_end_client(QObject* parent = nullptr);
    void set_server_config(const QString& ip, quint16 port);
	void start();           //启动后端并连接到服务器
	QString server_ip() const { return m_server_ip; }
	quint16 server_port() const { return m_server_port; }

	void send_message(const QJsonObject& obj);              //向后端发送消息
    void receive_message(const QJsonObject& obj);           //接收后端发送过来的消息
    static QString generateUniqueRequestId();
private slots:
    void onConnected();
    void onReadyRead();
    void on_try_connect();
private:
    bool is_server_running();
    void start_server();        //启动后端

signals:
    void post_update_camera_status(const QVariant& data);       //建立连接之后发送给界面，更新相机列表+显示打开的相机
    void post_update_camera_list(const QVariant& data);         //更新相机列表
    void post_camera_opened_success(const QVariant& data);      //后端成功打开相机，更新参数界面和图标状态
	void post_camera_closed_success();      	                //后端成功关闭相机，更新参数界面和图标状态

	void post_camera_parameter_changed_success(const QVariant& data);   //后端成功修改相机参数，更新参数界面和状态
	void post_camera_grab_set_success(const QVariant& data);            //后端成功设置采集状态
	void post_camera_trigger_once_success(const QVariant& data);        //后端成功触发采图，返回图像元数据

	void post_server_error(const QString& error_message);     //后端返回错误信息

private:
    QTcpSocket m_socket;
    QString m_server_ip{ "127.0.0.1" };
    quint16 m_server_port{ 5555 };

	QTimer m_retry_timer;           //定时器，启动服务之后可能需要一段时间才能启动完成，
									//设置一个定时器来检测后端服务是否启动完成并建立连接
    int m_retry_count{ 0 };
	int m_max_retry{ 20 };          // 最大重试次数
};
