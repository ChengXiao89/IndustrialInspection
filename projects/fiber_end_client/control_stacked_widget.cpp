#include "control_stacked_widget.h"

control_stacked_widget::control_stacked_widget(QWidget* parent)
	:QStackedWidget(parent)
{

}

control_stacked_widget::~control_stacked_widget()
{

}

void control_stacked_widget::initialize()
{
	setMinimumWidth(210);
	m_camera_parameter_widget = new camera_parameter_widget(this);
	m_camera_parameter_widget->initialize();
	addWidget(m_camera_parameter_widget);
	m_control_fiber_end_pane = new control_fiber_end_pane(this);
	m_control_fiber_end_pane->initialize();
	addWidget(m_control_fiber_end_pane);
	setCurrentIndex(0);
}

void control_stacked_widget::update_camera_parameter(const QJsonObject& obj)
{
	m_camera_parameter_widget->update_camera_parameter(obj);
	//setCurrentIndex(0);		// 切换到相机参数页面
}
void control_stacked_widget::reset_camera_parameter()
{
	m_camera_parameter_widget->reset_camera_parameter();
}

void control_stacked_widget::update_camera_grab_status(const QJsonObject& obj)
{
	m_camera_parameter_widget->update_camera_grab_status(obj);
	setCurrentIndex(0); // 切换到相机参数页面
}

void control_stacked_widget::update_fiber_end_pane_parameter(const QJsonObject& obj)
{
	m_control_fiber_end_pane->update_parameter(obj);
}

void control_stacked_widget::on_motion_parameter_changed_success(const QJsonObject& obj)
{
	m_control_fiber_end_pane->on_motion_parameter_changed_success(obj);
}

void control_stacked_widget::update_motion_position(int pos_x, int pos_y)
{
	m_control_fiber_end_pane->update_motion_position(pos_x, pos_y);
}