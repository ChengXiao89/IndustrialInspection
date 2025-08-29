/*********************************************
* 端面检测控制面板.
* 包含
* (1) 运控功能
		运动控制(运动到指定位置，运动指定步长)，设置光照亮度(暂不开放)，设置零点等
*		自动对焦(可选是否调用检测算法)
* (2) 参数设置
		Y 轴位置列表
		每张影像端面数量 自动检测开关
		存图路径
  (3) 运行功能
*		开始运行 : 指定 Y 轴位置列表，依次运动到这些位置之后执行自动对焦-检测
*		停止运行
**********************************************/
#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QCheckBox>

class control_fiber_end_pane : public QWidget
{
	Q_OBJECT
public:
	control_fiber_end_pane(QWidget* parent = nullptr);
	virtual ~control_fiber_end_pane() override;

	void initialize();
	QPushButton* create_push_button(const QSize& button_size, const QIcon& icon);		//辅助函数，创建一个按钮并为之设置图标
	QTableWidget* create_photo_locaion_table();											//辅助函数，创建Table控件显示拍照位置
	void update_photo_location_to_server();												//辅助函数，将Table中的位置列表更新到服务器

	void update_parameter(const QJsonObject& obj);		//连接服务器时服务器上存在已经打开的相机，或者在打开相机的时候调用，更新所有参数
	void on_motion_parameter_changed_success(const QJsonObject& obj);	//向服务器发送请求更改运控参数(光源亮度、运动速度、零点和步长)时响应服务器消息
	void update_motion_position(int pos_x, int pos_y);					//向服务器发送请求移动相机之后服务器会返回相机位置，在界面上更新显示
	void on_calibration_success();										//向服务器发送请求清晰度标定之后服务器会返回信息，在界面上更新按钮状态
	void on_anomaly_detection_finish(const QJsonObject& obj);			//向服务器发送请求异常检测之后服务器会返回信息，在界面上更新按钮状态

private slots:
	void on_light_brightness_changed();		//调整光源亮度
	void on_move_speed_changed();			//调整运动速度
	void on_move_step_x_changed();			//调整运动步长-X
	void on_move_step_y_changed();			//调整运动步长-Y

	void on_move_to_position();				//移动到编辑框中指定位置
	void on_set_current_position_zero();	//将当前位置设为零点
	void on_reset_position();				//相机复位.将相机移动到负限位并设为零点
	void on_move_forward_y();				//沿Y轴正向移动指定距离
	void on_move_back_x();					//沿X轴反向移动指定距离
	void on_move_forward_x();				//沿X轴正向移动指定距离
	void on_move_back_y();					//沿Y轴反向移动指定距离
	void on_auto_focus();					//自动对焦，调试使用
	void on_anomaly_detection();			//异常检测
	void on_calibration();					//标定清晰度曲线，设备安装完毕之后需要初始化调用一次，或者在曝光时间、增益、影像尺寸变化时需要标定
	void on_add_photo_location();								//添加拍照位置
	void on_remove_photo_location();							//移除拍照位置
	void on_fiber_end_count_changed();			//修改端面数量
	void on_auto_detect_set_changed(int check_state);	//设置自动检测开关
	void on_set_image_save_path();			//设置影像保存路径
	void on_start();
	void on_stop();
signals:
	void post_move_camera(const QJsonObject& obj);				//运动控制，移动相机
	void post_set_motion_parameter(const QJsonObject& obj);		//运动控制，设置运动参数(速度、步长、光源亮度，设为零点)
	void post_auto_focus();										//自动对焦，调试使用
	void post_anomaly_detection();								//异常检测
	void post_calibration();									//清晰度标定
	void post_update_server_parameter(const QJsonObject& obj);	//更新服务器参数，位置列表，端面数量，自动检测开关，保存位置
private:
	/******************* 运动控制 *******************/
	int m_light_brightness{ 0 };								// 光源亮度
	QLineEdit* m_edit_light_brightness{ nullptr };				// 显示并设置光源亮度
	int m_move_peed{ 0 };										// 运动速度
	QLineEdit* m_edit_move_speed{ nullptr };					// 显示并设置运动速度
	int m_position_x{ 0 }, m_position_y{ 0 };					// 设备位置
	QLineEdit* m_edit_position_x{ nullptr };					// 显示设备当前位置，可编辑并移动到编辑的位置
	QLineEdit* m_edit_position_y{ nullptr };
	QPushButton* m_push_button_move_to_position{ nullptr };		// 移动设备到指定位置
	QPushButton* m_push_button_set_position_zero{ nullptr };	// 将当前位置设置为零点
	QPushButton* m_push_button_reset_position{ nullptr };		// 相机复位

	int m_move_step_x{ 0 }, m_move_step_y{ 0 };							// 移动步长
	QLineEdit* m_edit_move_step_x{ nullptr };						// 设置移动步长
	QLineEdit* m_edit_move_step_y{ nullptr };
	QPushButton* m_push_button_move_forward_x{ nullptr };		// 向指定方向移动指定距离
	QPushButton* m_push_button_move_back_x{ nullptr };	
	QPushButton* m_push_button_move_forward_y{ nullptr };
	QPushButton* m_push_button_move_back_y{ nullptr };

	//QPushButton* m_push_button_auto_focus{ nullptr };		//自动对焦(包含拍照取图功能)
	QPushButton* m_push_button_calibration{ nullptr };		//标定
	QPushButton* m_push_button_anomaly_detection{ nullptr };//检测功能,在用户移动到指定位置并取图之后，对拍摄的影像执行自动对焦--异常检测操作，然后将结果显示在界面上
	/******************* 参数设置 *******************/
	// 显示服务器上的配置参数, 这里支持修改然后更新到服务器
	// 显示服务器上设置的位置列表，能够添加、删除和修改. 添加按钮不触发修改，只会向列表中添加一个 -1 项. 在修改和删除时才会触发修改操作
	QTableWidget* m_photo_location_table{ nullptr };					//拍照位置列表，不支持编辑修改	
	QPushButton* m_push_button_add_photo_location{ nullptr };			//添加拍照位置
	QPushButton* m_push_button_remove_photo_location{ nullptr };		//移除拍照位置
	int m_fiber_end_count{ 1 };											//影像端面个数
	QLineEdit* m_edit_fiber_count{ nullptr };							//显示并设置影像端面个数
	int m_auto_detect{ 0 };
	QCheckBox* m_check_auto_detect{ nullptr };							//自动检测开关
	QLineEdit* m_edit_image_save_path{ nullptr };						//影像端面个数
	QPushButton* m_push_button_set_image_save_path{ nullptr };			//设置影像保存路径
	/******************* 运行功能 *******************/
	QPushButton* m_push_button_start{ nullptr };		//开始运行
	QPushButton* m_push_button_stop{ nullptr };			//停止运行


};
