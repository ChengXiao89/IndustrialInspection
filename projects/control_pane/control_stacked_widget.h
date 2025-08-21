#pragma once
#include <QStackedWidget>

#include "camera_parameter_widget.h"
#include "algorithm_parameter_widget.h"

class control_stacked_widget : public QStackedWidget
{
	Q_OBJECT
public:
	control_stacked_widget(QWidget* parent = nullptr);
	~control_stacked_widget();

	void initialize();		//初始化

	void set_camera(const QVariant& data);		//设置相机参数

private:
	camera_parameter_widget* m_camera_parameter_widget{ nullptr };
	algorithm_parameter_widget* m_algorithm_parameter_widget{ nullptr };
};
