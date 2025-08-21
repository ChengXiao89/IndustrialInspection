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
	m_algorithm_parameter_widget = new algorithm_parameter_widget(this);
	m_algorithm_parameter_widget->initialize();
	addWidget(m_algorithm_parameter_widget);
	setCurrentIndex(0);
}

void control_stacked_widget::set_camera(const QVariant& data)
{
	m_camera_parameter_widget->set_camera(data);
}