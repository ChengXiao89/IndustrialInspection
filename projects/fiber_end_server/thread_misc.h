/********************
 * 其他任务线程,打开相机、设置相机参数、对焦、拍照等
 ********************/

#pragma once

#include "work_threads.h"
#include "device_manager.hpp"
#include "../device_camera/camera_factory.h"
#include "../common/image_shared_memory.h"

class thread_misc : public thread_base
{
public:
    thread_misc(QString name, QObject* parent = nullptr);
	virtual ~thread_misc() override;
	void set_device_manager(device_manager* manager) { m_device_manager = manager; }

	interface_camera* camera() const { return m_camera; } //获取相机对象)
	//将相机参数转换为 JSON 对象，发送给前端
	static QJsonObject camera_parameter_to_json(interface_camera* camera);
	static QJsonObject range_to_json(const st_range& range);
protected:
    void process_task(const QVariant& task_data) override;

private:
	interface_camera* m_camera{ nullptr }; //相机对象，用于执行打开相机、设置参数等操作
	device_manager* m_device_manager{ nullptr };	//设备管理器，用于存储和管理设备信息
	image_shared_memory m_image_shared_memory{ "my_image_channel" };
};