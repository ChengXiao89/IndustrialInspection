#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_fiber_end_client.h"

#include "camera_view.h"
#include "control_pane_dock_widget.h"
#include "central_widget.h"

/********************
 *端面检测客户端主窗口
 *前后端分离架构
 *********************/

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    main_window(QWidget *parent = nullptr);
    ~main_window();

    void closeEvent(QCloseEvent* event) override;

	void initialize();
	void initialize_actions();          //初始化工具栏动作和信号槽
    void initialize_widgets();          //初始化各个子窗口
	void initialize_camera_view();      //初始化相机视图
	void initialize_control_pane();     //初始化控制面板
	void initialize_central_widgets();  //初始化中心窗口视图


private slots:
    void on_new_connection();
	void on_disconnect();
	void on_stop_server();

    void on_camera_opened_success(const QJsonObject& obj);		//相机窗口打开相机之后发送消息，显示控制面板并显示相机参数
	void on_camera_closed_success();							//相机窗口关闭相机之后发送消息，隐藏控制面板
	void on_camera_trigger_once_success(QImage image);			//控制面板采图成功之后发送消息，显示采图结果

	void on_server_report_error(const QString& error_message);      //服务器返回错误信息          

private:
    fiber_end_client m_client;
    Ui::fiber_end_clientClass ui;
    QToolBar* m_toolBar{ nullptr };
	camera_view_dock_widget* m_camera_view{ nullptr };		//相机视图窗口
	control_pane_dock_widget* m_control_pane{ nullptr };	//控制面板窗口
	QStackedWidget* m_stacked_widget{ nullptr };		//堆栈窗口，用于显示中心窗口
	central_widget* m_central_widget{ nullptr }; //中心窗口视图
};

