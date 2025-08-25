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
#include <QListWidget>
#include <QCheckBox>

class control_fiber_end_pane : public QWidget
{
	Q_OBJECT
public:
	control_fiber_end_pane(QWidget* parent = nullptr);
	virtual ~control_fiber_end_pane() override;

	void initialize();
	QPushButton* create_push_button(const QSize& button_size, const QIcon& icon);		//辅助函数，创建一个按钮并为之设置图标

	void update_parameter(const QJsonObject& obj);
private slots:
	void on_move_to_position();				//移动到编辑框中指定位置
	void on_set_current_position_zero();	//将当前位置设为零点
	void on_move_forward_y();				//沿Y轴正向移动指定距离
	void on_move_back_x();					//沿X轴反向移动指定距离
	void on_move_forward_x();				//沿X轴正向移动指定距离
	void on_move_back_y();					//沿Y轴反向移动指定距离
	void on_auto_focus();					//自动对焦
	void on_add_y_position();				//添加 Y 轴位置
	void on_remove_y_position();			//移除 Y 轴位置
	void on_auto_detect_set_changed(int check_state);	//设置自动检测开关
	void on_set_image_save_path();			//设置影像保存路径
	void on_start();
	void on_stop();
signals:
	
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

	int m_move_step_x{ 0 }, m_move_step_y{ 0 };							// 移动步长
	QLineEdit* m_edit_move_step_x{ nullptr };						// 设置移动步长
	QLineEdit* m_edit_move_step_y{ nullptr };
	QPushButton* m_push_button_move_forward_x{ nullptr };		// 向指定方向移动指定距离
	QPushButton* m_push_button_move_back_x{ nullptr };	
	QPushButton* m_push_button_move_forward_y{ nullptr };
	QPushButton* m_push_button_move_back_y{ nullptr };

	QPushButton* m_push_button_auto_focus{ nullptr };		//自动对焦(包含拍照取图功能)
	QPushButton* m_push_button_detect{ nullptr };			//检测功能
	/******************* 参数设置 *******************/
	// 显示服务器上的配置参数, 这里支持修改然后更新到服务器
	// 显示服务器上设置的位置列表，能够添加、删除和修改. 添加按钮不触发修改，只会向列表中添加一个 -1 项. 在修改和删除时才会触发修改操作
	QListWidget* m_position_list{ nullptr };							//显示列表，双击修改	
	QPushButton* m_push_button_add_position{ nullptr };					//添加位置
	QPushButton* m_push_button_remove_position{ nullptr };				//移除位置
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
