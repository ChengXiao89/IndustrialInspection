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
	void update_camera_grab_status(bool start);				//更新相机采集状态

	void update_fiber_end_pane_parameter(const QJsonObject& obj);	//更新端面检测界面参数

	void on_motion_parameter_changed_success(const QJsonObject& obj);	//更新运动控制参数
	void update_motion_position(int pos_x, int pos_y);		//在端面检测界面更新相机位置
	void on_calibration_success();							//清晰度标定完成之后重置相关按钮状态
	void on_anomaly_detection_finish(const QJsonObject& obj);	//对拍摄的一张影像检测失败之后弹出提示消息并重置相关按钮状态
private:
	camera_parameter_widget* m_camera_parameter_widget{ nullptr };
	control_fiber_end_pane* m_control_fiber_end_pane{ nullptr };
};
