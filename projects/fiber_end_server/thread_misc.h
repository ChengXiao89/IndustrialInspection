/********************
 * 其他任务线程,打开相机、设置相机参数、对焦、拍照等
 ********************/

#pragma once

#include "work_threads.h"
#include "device_manager.hpp"
#include "motion_control.h"
#include "config.hpp"
#include "../device_camera/camera_factory.h"
#include "../common/image_shared_memory.h"
#include "auto_focus.h"
#include "../basic_algorithm/fiber_end_algorithm.h"

class thread_misc : public thread_base
{
public:
    thread_misc(QString name, QObject* parent = nullptr);
	virtual ~thread_misc() override;
	void set_device_manager(device_manager* manager) { m_device_manager = manager; }

	interface_camera* camera() const { return m_camera; }						//获取相机对象)
	st_config_data* config_data() const { return m_config_data; }				//获取配置参数
	motion_control* get_motion_control() const { return m_motion_control; }	//获取运控模块

	/***********************
	 * 初始化功能，包括初始化运控模块、自动对焦对象模块和自动检测模块
	 ***********************/
	bool initialize(st_config_data* config_data);

	bool setup_motion_control(st_config_data* config_data);					//启动运控模块

	//将相机参数转换为 JSON 对象，发送给前端
	static QJsonObject camera_parameter_to_json(interface_camera* camera);
	static QJsonObject range_to_json(const st_range& range);

protected:
    void process_task(const QVariant& task_data) override;
private:
	interface_camera* m_camera{ nullptr };					//相机对象，用于执行打开相机、设置参数等操作
	motion_control* m_motion_control{ nullptr };			//运控对象，用于移动相机
	auto_focus* m_auto_focus{ nullptr };					//自动对焦模块
	device_manager* m_device_manager{ nullptr };			//设备管理器，用于存储和管理设备信息
	st_config_data* m_config_data{ nullptr };				//服务配置参数,存储一些配置信息，例如拍照位置，保存路径，每张影像上的端面数量等
	fiber_end_algorithm* m_fiber_end_detector{ nullptr };		//端面检测器，指定影像数据，输出检测结果
	image_shared_memory m_shared_memory_trigger_image{ "trigger_image" };	// 共享内存对象，用于传输拍照得到的图像数据
	image_shared_memory m_shared_memory_detect_image{ "detect_image" };		// 共享内存对象，用于传输检测的图像数据
};