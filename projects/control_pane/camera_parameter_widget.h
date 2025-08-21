/***********************************
 * 相机参数-控制面板，在打开相机之后可见
 *********************************/

#pragma once
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

#include "../device_camera/interface_camera.h"

class camera_parameter_widget : public QWidget
{
	Q_OBJECT
public:
	camera_parameter_widget(QWidget* parent = nullptr);
	virtual ~camera_parameter_widget() override;

	void initialize();

	void set_camera(const QVariant& data);		//设置相机参数
	
private slots:
	void on_fps_changed();						//设置采集帧率
	void on_start_x_changed();					//设置 X 方向起点
	void on_start_y_changed();					//设置 Y 方向起点
	void on_width_changed();					//设置宽度
	void on_height_changed();					//设置高度
	void on_pixel_format_changed(int index);	//设置像素格式
	void on_exposure_set_changed(int index);	//设置曝光模式
	void on_auto_exposure_floor_changed();		//设置自动曝光时间下限
	void on_auto_exposure_upper_changed();		//设置自动曝光时间上限
	void on_exposure_time_changed();			//设置曝光时间
	void on_gain_set_changed(int index);		//设置增益模式
	void on_auto_gain_floor_changed();			//设置自动增益下限
	void on_auto_gain_upper_changed();			//设置自动增益上限
	void on_gain_changed();						//设置增益

	void on_trigger_mode_changed(int index);	//设置触发模式
	void on_trigger_source_changed(int index);	//设置触发源
	void on_start_grab();						//开始采集
	void on_trigger();							//采集一次(设置为触发模式时有效。连续模式不可用)
	void on_stop_grab();						//停止采集

signals:
	void post_trigger_finished(const QVariant& data);	//采图之后向主程序发送消息

private:
	void reset_parameters() const;				//当前相机参数为空时，重置界面所有参数

	interface_camera* m_camera{ nullptr };		//当前相机,这里负责设置参数以及发送命令，不负责资源释放
	bool m_updated_from_code{ false };	//标识变量，重置控件状态或者初始化相机参数时也会发送消息修改设备，需要进行屏蔽
	//用于 显示/设置 相机参数的控件，后续可能会扩展或移除
	//打开相机之后，获取相机参数并更新这些控件
	QDoubleSpinBox* m_fps_spin_box{ nullptr };				//采集帧率
	QSpinBox* m_max_width_spin_box{ nullptr };				//(影像)最大宽度
	QSpinBox* m_max_height_spin_box{ nullptr };				//(影像)最大高度
	QSpinBox* m_start_x_spin_box{ nullptr };				//影像起点偏移(X)
	QSpinBox* m_start_y_spin_box{ nullptr };				//影像起点偏移(Y)
	QSpinBox* m_width_spin_box{ nullptr };					//影像宽度
	QSpinBox* m_height_spin_box{ nullptr };					//影像高度
	QComboBox* m_pixel_format_combo_box{ nullptr };			//像素格式
	QComboBox* m_auto_exposure_combo_box{ nullptr };		//自动曝光
	QDoubleSpinBox* m_auto_exposure_floor_spin_box{ nullptr };		//自动曝光时间下限(开启自动曝光时可用)
	QDoubleSpinBox* m_auto_exposure_upper_spin_box{ nullptr };		//自动曝光时间上限(开启自动曝光时可用)
	QDoubleSpinBox* m_exposure_time_spin_box{ nullptr };			//曝光时间(关闭自动曝光时可用)
	QComboBox* m_auto_gain_combo_box{ nullptr };			//自动增益
	QDoubleSpinBox* m_auto_gain_floor_spin_box{ nullptr };	//自动增益下限（开启自动增益时可用）
	QDoubleSpinBox* m_auto_gain_upper_spin_box{ nullptr };	//自动增益上限（开启自动增益时可用）
	QDoubleSpinBox* m_gain_spin_box{ nullptr };				//增益（关闭自动增益时可用）

	//相机控制控件，采集模式和执行采图
	QComboBox* m_trigger_mode_combo_box{ nullptr };
	QComboBox* m_trigger_source_combo_box{ nullptr };
	QPushButton* m_start_capture{ nullptr };
	QPushButton* m_run_trigger{ nullptr };
	QPushButton* m_stop_capture{ nullptr };
	void set_controls_status();
};
 