#include "thread_misc.h"
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QBuffer>
#include <QImage>




thread_misc::thread_misc(QString name, QObject* parent)
    :thread_base(name, parent)
{

}

thread_misc::~thread_misc()
{
	if (m_camera != nullptr)
	{
		m_camera->close();
		delete m_camera;        // 确保释放相机资源
		m_camera = nullptr;     // 避免悬空指针
	}
}

void thread_misc::process_task(const QVariant& task_data)
{
    QJsonObject obj = task_data.toJsonObject();
    QString command = obj["command"].toString();
	QJsonObject result_obj;     //返回的消息对象
    if (command == "client_request_open_camera")
    {
		QString unique_id = obj["param"].toString();
		st_device_info* device_info = m_device_manager->get_device_info(unique_id);
        if(device_info == nullptr)
        {
            result_obj["command"] = "server_report_error";
            result_obj["param"] = QString("未找到相机设备: %1").arg(unique_id);
            result_obj["request_id"] = obj["request_id"];
		}
        else
        {
            m_camera = camera_factory::create_camera(device_info);
            if(m_camera == nullptr)
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("无法创建相机设备: %1").arg(unique_id);
                result_obj["request_id"] = obj["request_id"];
			}
            else
            {
                if(m_camera->open() != STATUS_SUCCESS)
                {
                    result_obj["command"] = "server_report_error";
                    result_obj["param"] = QString("无法打开相机设备: %1").arg(unique_id);
                    result_obj["request_id"] = obj["request_id"];
                }
                else
                {
                    result_obj["command"] = "server_camera_opened_success";
                    QJsonObject param_obj = camera_parameter_to_json(m_camera);
                    result_obj["param"] = param_obj;        // 将相机参数转换为 JSON 对象
					result_obj["unique_id"] = unique_id;    // 返回相机的唯一标识符
                    result_obj["request_id"] = obj["request_id"];
                }
            }
        }
        emit post_task_finished(QVariant::fromValue(result_obj));
    }
    else if(command == "client_request_close_camera")
    {
        if(m_camera != nullptr)
        {
            m_camera->close();
        }
        result_obj["command"] = "server_camera_closed_success";
        result_obj["request_id"] = obj["request_id"];
        emit post_task_finished(QVariant::fromValue(result_obj));
    }
    else if (command == "client_request_change_camera_parameter")
    {
        if (m_camera != nullptr)
        {
            QJsonObject param_obj = obj["param"].toObject();
            QString name = param_obj["name"].toString();
			int ret = STATUS_SUCCESS;
            if (name == "fps")
            {
                ret = m_camera->set_frame_rate(param_obj["value"].toDouble());
            }
            else if (name == "start_x")
            {
                ret = m_camera->set_start_x(param_obj["value"].toInt());
            }
            else if (name == "start_y")
            {
                ret = m_camera->set_start_y(param_obj["value"].toInt());
            }
            else if (name == "width")
            {
                ret = m_camera->set_width(param_obj["value"].toInt());
            }
            else if (name == "height")
            {
                ret = m_camera->set_height(param_obj["value"].toInt());
            }
            else if (name == "pixel_format")
            {
                ret = m_camera->set_pixel_format(param_obj["value"].toString());
            }
            else if (name == "auto_exposure_mode")
            {
                ret = m_camera->set_auto_exposure_mode(param_obj["value"].toString());
            }
            else if (name == "auto_exposure_time_floor")
            {
                ret = m_camera->set_auto_exposure_time_floor(param_obj["value"].toDouble());
            }
            else if (name == "auto_exposure_time_upper")
            {
                ret = m_camera->set_auto_exposure_time_upper(param_obj["value"].toDouble());
            }
            else if (name == "exposure_time")
            {
                ret = m_camera->set_exposure_time(param_obj["value"].toDouble());
            }
            else if (name == "auto_gain_mode")
            {
                ret = m_camera->set_auto_gain_mode(param_obj["value"].toString());
            }
            else if(name == "auto_gain_floor")
            {
                ret = m_camera->set_auto_gain_floor(param_obj["value"].toDouble());
            }
            else if (name == "auto_gain_upper")
            {
                ret = m_camera->set_auto_gain_upper(param_obj["value"].toDouble());
            }
            else if (name == "gain")
            {
                ret = m_camera->set_gain(param_obj["value"].toDouble());
            }
            else if (name == "trigger_mode")
            {
                ret = m_camera->set_trigger_mode(param_obj["value"].toString());
            }
            else if (name == "trigger_source")
            {
                ret = m_camera->set_trigger_source(param_obj["value"].toString());
            }
            if(ret != STATUS_SUCCESS)
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("设置相机参数失败: %1").arg(m_camera->map_ret_status(ret));
                result_obj["request_id"] = obj["request_id"];
                emit post_task_finished(QVariant::fromValue(result_obj));
            }
            else
            {
                // 成功后重新获取相机参数并返回
                QJsonObject camera_obj = camera_parameter_to_json(m_camera);
                result_obj["command"] = "server_camera_parameter_changed_success";
                result_obj["param"] = camera_obj;
                result_obj["request_id"] = obj["request_id"];
                emit post_task_finished(QVariant::fromValue(result_obj));
			}
        }
        else
        {
            result_obj["command"] = "server_report_error";
            result_obj["param"] = QString("相机对象无效！");
            result_obj["request_id"] = obj["request_id"];
            emit post_task_finished(QVariant::fromValue(result_obj));
        }
    }
    else if(command == "client_request_start_grab")
    {
        if(m_camera != nullptr)
        {
			bool start = obj["param"].toBool();
            int ret = STATUS_SUCCESS;
            QString report_info("");
            if(start)
            {
                ret = m_camera->start_grab();
                if(ret != STATUS_SUCCESS)
                {
                    report_info = QString("开始采集失败: %1").arg(m_camera->map_ret_status(ret));
				}
            }
            else
            {
				ret = m_camera->stop_grab();
                if (ret != STATUS_SUCCESS)
                {
                    report_info = QString("停止采集失败: %1").arg(m_camera->map_ret_status(ret));
                }
            }
            
            if(ret != STATUS_SUCCESS)
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = report_info;
            }
            else
            {
                result_obj["command"] = "server_camera_grab_set_success";
                result_obj["param"] = start;
            }
            result_obj["request_id"] = obj["request_id"];
            emit post_task_finished(QVariant::fromValue(result_obj));
        }
        else
        {
            result_obj["command"] = "server_report_error";
            result_obj["param"] = QString("相机对象无效！");
            result_obj["request_id"] = obj["request_id"];
            emit post_task_finished(QVariant::fromValue(result_obj));
        }
    }
    else if(command == "client_request_trigger_once")
    {
        result_obj["task_type"] = TASK_TYPE_TRIGGER_ONCE; // 设置任务类型为采集任务
        if(m_camera != nullptr)
        {
            QImage img = m_camera->trigger_once();
            if(img.isNull())
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("触发采图失败");
            }
            else
            {
                //测试保存图片
                if(0)
                {
                    img.save("D:/Temp/server.png");
                }
                st_image_meta meta;
                if (!m_image_shared_memory.write_image(img, meta))
                {
                    result_obj["command"] = "server_report_error";
                    result_obj["param"] = QString("写入图片数据失败");
                }
                else
                {
                    result_obj["command"] = "server_camera_trigger_once_success";
                    QJsonObject json = image_shared_memory::meta_to_json(meta);
					result_obj["param"] = json;
                }
            }
            result_obj["request_id"] = obj["request_id"];
            emit post_task_finished(QVariant::fromValue(result_obj));
        }
	}
}

QJsonObject thread_misc::camera_parameter_to_json(interface_camera* camera)
{
    QJsonObject root;
	//唯一标识符
    root["unique_id"] = camera->m_unique_id;
	// 基本采集参数
	double fps = camera->get_frame_rate();
    root["frame_rate"] = fps;
    root["frame_rate_range"] = range_to_json(camera->get_frame_rate_range());
	int max_width = camera->get_maximum_width();
    root["maximum_width"] = max_width;
    root["maximum_height"] = camera->get_maximum_height();
    root["start_x"] = camera->get_start_x();
    root["start_x_range"] = range_to_json(camera->get_start_x_range());
    root["start_y"] = camera->get_start_y();
    root["start_y_range"] = range_to_json(camera->get_start_y_range());
    root["width"] = camera->get_width();
    root["width_range"] = range_to_json(camera->get_width_range());
    root["height"] = camera->get_height();
    root["height_range"] = range_to_json(camera->get_height_range());

    // 像素格式
    {
        QJsonArray fmtArray;
        for (auto it = camera->enum_pixel_format().begin(); it != camera->enum_pixel_format().end(); ++it)
        {
            QJsonObject fmt;
            fmt["name"] = it.key();
            fmt["value"] = static_cast<int>(it.value());
            fmtArray.append(fmt);
        }
        root["pixel_formats"] = fmtArray;
        root["current_pixel_format"] = camera->get_pixel_format();
    }

    // 自动曝光
    {
        QJsonArray modes;
        for (auto it = camera->enum_supported_auto_exposure_mode().begin();
            it != camera->enum_supported_auto_exposure_mode().end(); ++it)
        {
            QJsonObject mode;
            mode["name"] = it.key();
            mode["value"] = static_cast<int>(it.value());
            modes.append(mode);
        }
        root["auto_exposure_modes"] = modes;
        root["current_auto_exposure_mode"] = camera->get_auto_exposure_mode();
        root["is_auto_exposure_closed"] = camera->is_auto_exposure_closed();
        root["auto_exposure_time_floor"] = camera->get_auto_exposure_time_floor();
        root["auto_exposure_time_floor_range"] = range_to_json(camera->get_auto_exposure_time_floor_range());
        root["auto_exposure_time_upper"] = camera->get_auto_exposure_time_upper();
        root["auto_exposure_time_upper_range"] = range_to_json(camera->get_auto_exposure_time_upper_range());
        root["exposure_time"] = camera->get_exposure_time();
        root["exposure_time_range"] = range_to_json(camera->get_exposure_time_range());
    }

    // 自动增益
    {
        QJsonArray modes;
        for (auto it = camera->enum_supported_auto_gain_mode().begin();
            it != camera->enum_supported_auto_gain_mode().end(); ++it)
        {
            QJsonObject mode;
            mode["name"] = it.key();
            mode["value"] = static_cast<int>(it.value());
            modes.append(mode);
        }
        root["auto_gain_modes"] = modes;
        root["current_auto_gain_mode"] = camera->get_auto_gain_mode();
        root["is_auto_gain_closed"] = camera->is_auto_gain_closed();
        root["auto_gain_floor"] = camera->get_auto_gain_floor();
        root["auto_gain_floor_range"] = range_to_json(camera->get_auto_gain_floor_range());
        root["auto_gain_upper"] = camera->get_auto_gain_upper();
        root["auto_gain_upper_range"] = range_to_json(camera->get_auto_gain_upper_range());
        root["gain"] = camera->get_gain();
        root["gain_range"] = range_to_json(camera->get_gain_range());
    }

    // 触发模式
    {
        QJsonArray triggerModes;
        for (auto it = camera->enum_supported_trigger_mode().begin();
            it != camera->enum_supported_trigger_mode().end(); ++it)
        {
            QJsonObject tm;
            tm["name"] = it.key();
            tm["value"] = static_cast<int>(it.value());
            triggerModes.append(tm);
        }
        root["trigger_modes"] = triggerModes;
        root["current_trigger_mode"] = camera->get_trigger_mode();
    }

    // 触发源
    {
        QJsonArray triggerSources;
        for (auto it = camera->enum_supported_trigger_source().begin();
            it != camera->enum_supported_trigger_source().end(); ++it)
        {
            QJsonObject ts;
            ts["name"] = it.key();
            ts["value"] = static_cast<int>(it.value());
            triggerSources.append(ts);
        }
        root["trigger_sources"] = triggerSources;
        root["current_trigger_source"] = camera->get_trigger_source();
    }

	root["is_grab_running"] = camera->is_grab_running();    //是否正在采集

    return root;
}

QJsonObject thread_misc::range_to_json(const st_range& range)
{
    QJsonObject obj;
    obj["min"] = range.min;
    obj["max"] = range.max;
    return obj;
}
