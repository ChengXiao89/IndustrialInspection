#include "window.h"

#include <QMessageBox>

#include "new_connection_dlg.h"

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    resize(1280, 720);
	initialize();
}

main_window::~main_window()
{
	
}

void main_window::closeEvent(QCloseEvent* event)
{
	
}

void main_window::initialize()
{
	initialize_actions();
	initialize_widgets();
	//注册消息
	connect(&m_client, &fiber_end_client::post_server_error, this, &main_window::on_server_report_error);
}

void main_window::initialize_actions()
{
	connect(ui.action_new_connection, &QAction::triggered, this, &main_window::on_new_connection);
	connect(ui.action_disconnect, &QAction::triggered, this, &main_window::on_disconnect);
	connect(ui.action_stop_server, &QAction::triggered, this, &main_window::on_stop_server);

}

void main_window::initialize_widgets()
{
	initialize_camera_view();
	initialize_control_pane();
	initialize_central_widgets();
}

void main_window::initialize_camera_view()
{
	m_camera_view = new camera_view_dock_widget(&m_client, this);
	m_camera_view->initialize();
	m_camera_view->setMaximumWidth(240);
	//注册消息
	connect(m_camera_view, &camera_view_dock_widget::post_camera_opened_success, this, &main_window::on_camera_opened_success);
	connect(m_camera_view, &camera_view_dock_widget::post_camera_closed_success, this, &main_window::on_camera_closed_success);
	addDockWidget(Qt::LeftDockWidgetArea, m_camera_view);
}

void main_window::initialize_control_pane()
{
	m_control_pane = new control_pane_dock_widget(&m_client, this);
	m_control_pane->initialize();
	m_control_pane->setMaximumWidth(540);
	//注册消息
	connect(m_control_pane, &control_pane_dock_widget::post_camera_trigger_once_success, this, &main_window::on_camera_trigger_once_success);
	connect(m_control_pane, &control_pane_dock_widget::post_camera_moved_success, this, &main_window::on_camera_moved_success);
	connect(m_control_pane, &control_pane_dock_widget::post_anomaly_detection_once, this, &main_window::on_anomaly_detection_once);

	addDockWidget(Qt::RightDockWidgetArea, m_control_pane);
	//初始不显示,只有在打开相机之后才显示
	m_control_pane->hide();
}

void main_window::initialize_central_widgets()
{
	m_central_widget = new central_widget(&m_client,this);
	m_central_widget->initialize();
	m_stacked_widget = new QStackedWidget(this);
	m_stacked_widget->addWidget(m_central_widget);
	setCentralWidget(m_stacked_widget);
	m_central_widget->hide(); //初始不显示,只有进行采图之后才显示

}

void main_window::on_new_connection()
{
	new_connection_dlg dlg(m_client.server_ip(),m_client.server_port());
	dlg.exec();
	if(dlg.m_ret == 0)
	{
		m_client.set_server_config(dlg.m_server_ip, dlg.m_server_port);
		m_client.start();
	}
}

void main_window::on_disconnect()
{
	
}

void main_window::on_stop_server()
{
	
}

void main_window::on_camera_opened_success(const QJsonObject& obj)
{
	if(m_control_pane != nullptr)
	{
		m_control_pane->update_camera_parameter(obj);	//更新相机节目中的参数信息
		m_control_pane->update_fiber_end_pane_parameter(obj);	//更新端面检测界面中的参数信息
		m_control_pane->show();							//打开相机后显示控制面板
	}
}

void main_window::on_camera_closed_success()
{
	if (m_control_pane != nullptr)
	{
		m_control_pane->reset_camera_parameter(); //更新控制面板中的相机信息
		m_control_pane->hide(); //打开相机后显示控制面板
	}
	if(m_central_widget != nullptr)
	{
		m_central_widget->hide();				//关闭相机后隐藏中心窗口视图
	}
}

void main_window::on_camera_trigger_once_success(QImage image)
{
	if(m_central_widget != nullptr)
	{
		m_central_widget->set_image(image);
		m_central_widget->show(); //显示中心窗口视图
	}
}

void main_window::on_camera_moved_success(QImage image)
{
	if (m_central_widget != nullptr)
	{
		m_central_widget->set_image(image);
		m_central_widget->show(); //显示中心窗口视图
	}
}

void main_window::on_anomaly_detection_once(QImage image)
{
	if (m_central_widget != nullptr)
	{
		m_central_widget->add_detected_image(image);
		m_central_widget->show(); //显示中心窗口视图
	}
}

void main_window::on_server_report_error(const QString& error_message)
{
	QMessageBox::warning(this, "服务器错误", error_message);
}
