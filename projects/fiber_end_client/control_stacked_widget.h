#pragma once
#include <QStackedWidget>
#include "control_camera_parameter_widget.h"
#include "control_fiber_end_pane.h"

class control_stacked_widget : public QStackedWidget
{
	Q_OBJECT
public:
	control_stacked_widget(QWidget* parent = nullptr);
	~control_stacked_widget();

	void initialize();		//初始化

	void update_camera_parameter(const QJsonObject& obj);	//更新相机参数
	void reset_camera_parameter();							//重置相机参数
	void update_camera_grab_status(const QJsonObject& obj);	//更新相机采集状态
private:
	camera_parameter_widget* m_camera_parameter_widget{ nullptr };
	control_fiber_end_pane* m_control_fiber_end_pane{ nullptr };
};
