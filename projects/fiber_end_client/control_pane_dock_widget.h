#pragma once

#include <QDockWidget>

#include "control_tree_widget.h"
#include "control_stacked_widget.h"
#include "client.h"
#include "../common/image_shared_memory.h"

class control_pane_dock_widget : public QDockWidget
{
	Q_OBJECT
public:
	control_pane_dock_widget(fiber_end_client* client, QWidget* parent = nullptr);
	virtual ~control_pane_dock_widget() override;
	void initialize();				// 初始化控制面板视图

	void update_camera_parameter(const QJsonObject& obj);	// 更新相机参数
	void reset_camera_parameter();							// 重置相机参数

	void update_fiber_end_pane_parameter(const QJsonObject& obj);	//更新端面检测界面参数

private slots:
	/*******************接收子控件消息*********************/
	void on_request_trigger_once();								//子控件发送的触发一次采图请求
	void on_request_change_camera_parameter(const QJsonObject& obj);	//子控件发送的修改参数请求
	void on_request_start_grab(bool start);						//子控件发送的开始/停止采集请求

	void on_request_move_camera(const QJsonObject& obj);			//子控件发送的移动相机请求，包括前后左右移动和移动到指定位置
	void on_request_set_motion_parameter(const QJsonObject& obj);	//子控件发送的运控参数设置请求，包括设置光源亮度，运动速度，步长和零点
	void on_request_auto_focus();									//子控件发送的自动对焦请求
	void on_request_anomaly_detection();							//子控件发送的异常检测请求
	void on_request_calibration();									//子控件发送的清晰度标定请求
	void on_request_update_server_parameter(const QJsonObject& obj);//子控件发送的更新服务器参数请求(位置列表，端面数量，自动检测开关和保存路径)
	
	/*******************接收后端消息*********************/
	void on_camera_parameter_changed_success(const QVariant& obj);	// 接收后端的相机参数修改成功消息，更新界面
	void on_camera_grab_set_success(const QVariant& obj);			// 接收后端的采集状态修改成功消息，更新界面
	void on_camera_trigger_once_success(const QVariant& obj);		// 接收后端的采图成功消息，更新界面

	void on_camera_moved_success(const QVariant& obj);				// 接收后端相机移动成功的消息，更新界面，包括显示影像和更新相机位置
	void on_motion_parameter_changed_success(const QVariant& obj);	// 接收后端运控参数修改成功的消息，更新界面，包括光源亮度、运动速度、相机位置(置0成功)，移动步长等
	void on_anomaly_detection_once(const QVariant& obj);			// 接收后端完成一张对焦之后影像的异常检测的消息
	void on_calibration_success();									// 接收后端完成清晰度标定的消息
	void on_anomaly_detection_finish(const QVariant& obj);			// 接受后端检测完成的消息
signals:
	void post_camera_trigger_once_success(QImage img);				//采图成功,向主窗口发送信号打开中央控件显示影像
	void post_camera_moved_success(QImage img);						//移动成功，实际上与采图成功链接的是相同的槽函数，打开中央窗口控件并显示影像数据

	void post_anomaly_detection_once(QImage img);					//检测完成，向主窗口发送信号打开中央控件显示检测结果

private:
	fiber_end_client* m_client{ nullptr };  // 客户端对象，用于与后端通信
	control_tree_widget* m_control_tree_widget{ nullptr };
	control_stacked_widget* m_control_stacked_widget{ nullptr };
	image_shared_memory m_shared_memory_trigger_image{ "trigger_image" };	// 共享内存对象，用于传输拍照得到的图像数据
	image_shared_memory m_shared_memory_detect_image{ "detect_image" };		// 共享内存对象，用于传输检测的图像数据
};
