#include "thread_misc.h"
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QBuffer>
#include <QImage>

#include "../common/common_api.h"

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
    if(m_motion_control != nullptr)
    {
        delete m_motion_control;
        m_motion_control = nullptr;
    }
    if(m_auto_focus != nullptr)
    {
        delete m_auto_focus;
        m_auto_focus = nullptr;
    }
    if (m_fiber_end_detector != nullptr)
    {
        delete m_fiber_end_detector;
        m_fiber_end_detector = nullptr;
    }
}

bool thread_misc::initialize(st_config_data* config_data)
{
	if(!setup_motion_control(config_data))
	{
        std::cerr << "setup_motion_control fail!" << std::endl;
		return false;
	}
    //m_auto_focus = new AutoFocus(m_motion_control,m_camera,m_config_data->m_fiber_end_count);
    

    return true;
}

bool thread_misc::setup_motion_control(st_config_data* config_data)
{
    std::string port = "COM1";  // 串口号
    unsigned int baud_rate = 115200;
    m_config_data = config_data;
    m_motion_control = new motion_control(port, baud_rate);
    if(!m_motion_control->open_port())
    {
	    return false;
    }
    m_motion_control->reset(0);
    m_motion_control->reset(1);
    m_motion_control->get_position(m_config_data->m_position_x, m_config_data->m_position_y);
    m_motion_control->set_light_source_param(m_config_data->m_light_brightness, 80, 1);
	return true;
}

void thread_misc::process_task(const QVariant& task_data)
{
    QJsonObject obj = task_data.toJsonObject();
    QString command = obj["command"].toString();
	QJsonObject result_obj;     //返回的消息对象
    result_obj["request_id"] = obj["request_id"];
    if (command == "client_request_open_camera")
    {
		QString unique_id = obj["param"].toString();
		st_device_info* device_info = m_device_manager->get_device_info(unique_id);
        if(device_info == nullptr)
        {
            result_obj["command"] = "server_report_error";
            result_obj["param"] = QString("未找到相机设备: %1").arg(unique_id);
		}
        else
        {
            m_camera = camera_factory::create_camera(device_info);
            if(m_camera == nullptr)
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("无法创建相机设备: %1").arg(unique_id);
			}
            else
            {
                if(m_camera->open() != STATUS_SUCCESS)
                {
                    result_obj["command"] = "server_report_error";
                    result_obj["param"] = QString("无法打开相机设备: %1").arg(unique_id);
                }
                else
                {
                    result_obj["command"] = "server_camera_opened_success";
                    QJsonObject camera_obj = camera_parameter_to_json(m_camera);
                    result_obj["camera"] = camera_obj;        // 将相机参数转换为 JSON 对象
					result_obj["unique_id"] = unique_id;    // 返回相机的唯一标识符
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
                emit post_task_finished(QVariant::fromValue(result_obj));
            }
            else
            {
                // 成功后重新获取相机参数并返回
                QJsonObject camera_obj = camera_parameter_to_json(m_camera);
                result_obj["command"] = "server_camera_parameter_changed_success";
                result_obj["camera"] = camera_obj;
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
        }
        else
        {
            result_obj["command"] = "server_report_error";
            result_obj["param"] = QString("相机对象无效！");
        }
        emit post_task_finished(QVariant::fromValue(result_obj));
    }
    else if(command == "client_request_trigger_once")
    {
        result_obj["task_type"] = TASK_TYPE_TRIGGER_ONCE; // 设置任务类型为采集任务
        if (m_camera != nullptr)
        {
            QImage img = m_camera->trigger_once();
            if (img.isNull())
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("触发采图失败");
            }
            else
            {
                if (0)      //测试保存图片
                {
                    img.save("D:/Temp/server.png");
                }
                st_image_meta meta;
                if (!m_shared_memory_trigger_image.write_image(img, meta))
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
            emit post_task_finished(QVariant::fromValue(result_obj));
        }
	}
    else if(command == "client_request_move_camera")
    {
        result_obj["task_type"] = TASK_TYPE_MOVE_CAMERA;        //设置任务类型为移动相机任务(需要采图)
        result_obj["command"] = "server_move_camera_success";   //使用统一回复命令，涉及到移动+取图两个步骤，在result_obj["image"]中存储取图状态
        QJsonObject param = obj["param"].toObject();
        int pos_x(0), pos_y(0);
        if (param["name"] == "move_to_position")
        {
            if (m_motion_control != nullptr)
            {
                pos_x = param["x"].toInt();
                pos_y = param["y"].toInt();
                m_motion_control->move_position(1, pos_y, m_config_data->m_move_speed);
                m_motion_control->move_position(0, pos_x, m_config_data->m_move_speed);
                m_motion_control->get_position(pos_x, pos_y);
            }
        }
	    else if (param["name"] == "move_forward_y")
	    {
            if(m_motion_control != nullptr)
            {
                m_motion_control->move_distance(1, m_config_data->m_move_step_y, m_config_data->m_move_speed);
                m_motion_control->get_position(pos_x, pos_y);
            }
	    }
        else if (param["name"] == "move_back_x")
        {
        	if (m_motion_control != nullptr)
            {
                m_motion_control->move_distance(0, -m_config_data->m_move_step_x, m_config_data->m_move_speed);
                m_motion_control->get_position(pos_x, pos_y);
            }
        }
        else if (param["name"] == "move_forward_x")
        {
            if (m_motion_control != nullptr)
            {
                m_motion_control->move_distance(0, m_config_data->m_move_step_x, m_config_data->m_move_speed);
                m_motion_control->get_position(pos_x, pos_y);
            }
        }
        else if (param["name"] == "move_back_y")
        {
            if (m_motion_control != nullptr)
            {
                m_motion_control->move_distance(1, -m_config_data->m_move_step_y, m_config_data->m_move_speed);
                m_motion_control->get_position(pos_x, pos_y);
            }
        }
        result_obj["x"] = pos_x;
        result_obj["y"] = pos_y;
        //移动相机之后采图
        if (m_camera != nullptr)
        {
            //需要 触发模式+软触发
            m_camera->set_trigger_mode(global_trigger_mode_once);
            m_camera->set_trigger_source(global_trigger_source_software);
            m_camera->start_grab();
            QImage img = m_camera->trigger_once();
            if (img.isNull())
            {
                result_obj["image"] = "trigger error";
            }
            else
            {
                st_image_meta meta;
                if (!m_shared_memory_trigger_image.write_image(img, meta))
                {
                    result_obj["image"] = "write image error";
                }
                else
                {
                    result_obj["image"] = "success";
                    QJsonObject json = image_shared_memory::meta_to_json(meta);
                    result_obj["image_data"] = json;
                }
            }
            emit post_task_finished(QVariant::fromValue(result_obj));
        }

    }
    else if(command == "client_request_set_motion_parameter")
    {
        QJsonObject param = obj["param"].toObject();
        
    	if(param["name"] == "set_light_brightness")
        {
            int light_brightness = param["value"].toInt();
            bool ret = m_motion_control->set_light_source_param(light_brightness, 80, 1);
            if(ret)
            {
                m_config_data->m_light_brightness = light_brightness;
                m_config_data->save();
            }
            else
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("设置光源亮度失败!");
            }
        }
        else if(param["name"] == "set_move_speed")
        {
            int move_speed = param["value"].toInt();
            m_config_data->m_move_speed = move_speed;
            m_config_data->save();
        }
        else if (param["name"] == "set_move_step_x")
        {
            int move_step_x = param["value"].toInt();
            m_config_data->m_move_step_x = move_step_x;
            m_config_data->save();
        }
        else if (param["name"] == "set_move_step_y")
        {
            int move_step_y = param["value"].toInt();
            m_config_data->m_move_step_y = move_step_y;
            m_config_data->save();
        }
        else if (param["name"] == "set_zero")
        {
            bool ret = m_motion_control->set_current_position_zero(0);
            if(ret)
            {
                ret = m_motion_control->set_current_position_zero(1);
                if (ret)
                {
                    m_motion_control->get_position(m_config_data->m_position_x, m_config_data->m_position_y);
                    result_obj["command"] = "server_set_motion_parameter_success";
                    result_obj["name"] = param["name"];
                    result_obj["x"] = m_config_data->m_position_x;
                    result_obj["y"] = m_config_data->m_position_y;
                }
                else
                {
                    result_obj["command"] = "server_report_error";
                    result_obj["param"] = QString("设置零点失败!");
                }
            }
            else
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("设置零点失败!");
            }
        }
        else if (param["name"] == "reset_position")
        {
            bool ret = m_motion_control->reset(0);
            if (ret)
            {
                ret = m_motion_control->reset(1);
                if (ret)
                {
                    m_motion_control->get_position(m_config_data->m_position_x, m_config_data->m_position_y);
                    result_obj["command"] = "server_set_motion_parameter_success";
                    result_obj["name"] = param["name"];
                    result_obj["x"] = m_config_data->m_position_x;
                    result_obj["y"] = m_config_data->m_position_y;
                }
                else
                {
                    result_obj["command"] = "server_report_error";
                    result_obj["param"] = QString("复位失败!");
                }
            }
            else
            {
                result_obj["command"] = "server_report_error";
                result_obj["param"] = QString("复位失败!");
            }
        }
        emit post_task_finished(QVariant::fromValue(result_obj));
    }
    else if(command == "client_request_auto_focus")
    {
        if(m_auto_focus == nullptr)
        {
            m_auto_focus = new auto_focus(static_cast<void*>(this));
        }
        std::vector<cv::Mat> images = m_auto_focus->get_focus_images();
        if(1)
        {
            for (int i = 0; i < images.size();i++)
            {
                char szPath[256] = { 0 };
                sprintf_s(szPath, "C:/Temp/focus_%d.png", i);
                //QString file_path = QString("D:/Temp/focus_%1.png").arg(i);
                cv::imwrite(szPath,images[i]);
            }
        }
    }
    else if (command == "client_request_anomaly_detection")
    {
        if(m_fiber_end_detector == nullptr)
        {
            m_fiber_end_detector = new fiber_end_algorithm;
            QString current_directory = QCoreApplication::applicationDirPath();
            std::string onnx_model_path = (current_directory + "/weights/fiber_end_model.onnx").toStdString();
            std::string faiss_index_path = (current_directory + "/faiss_index/faiss-binary_0").toStdString();
            std::string shape_model_path = (current_directory + "/shape_model/model.bin").toStdString();
            if (!m_fiber_end_detector->initialize(onnx_model_path,faiss_index_path,shape_model_path))
            {
                delete m_fiber_end_detector;
                m_fiber_end_detector = nullptr;
                std::cerr << "fiber_end_algorithm initialize error!" << std::endl;
                result_obj["command"] = "server_anomaly_detection_finish";
                result_obj["param"] = QString::fromStdString("算法初始化失败!");
                emit post_task_finished(QVariant::fromValue(result_obj));
                return;
            }
        }
        if (m_auto_focus == nullptr)
        {
            m_auto_focus = new auto_focus(static_cast<void*>(this));
        }
        //得到若干清晰的单通道端面影像，每张影像上包含一个端面，使用算法进行检测
        std::vector<cv::Mat> images = m_auto_focus->get_focus_images();
        //对每张自动对焦的结果执行异常检测
        for (int i = 0;i < images.size();i++)
        {
            QJsonObject ret_obj;     //返回的消息对象
            ret_obj["request_id"] = obj["request_id"];
            ret_obj["command"] = "server_anomaly_detection_once";      //每检测一张就发送一次消息
            ret_obj["task_finish"] = false;
            //1.将影像转换成三通道，后续需要进行模型推理
            cv::Mat input_image;
            cvtColor(images[i], input_image, cv::COLOR_GRAY2BGR);
            if(0)
            {
                char sz_path[256] = { 0 };
                sprintf_s(sz_path, "C:/Temp/server_%d.png", i + 1);
                cv::imwrite(sz_path, input_image);
            }
            //2. 推理并得到结果
            m_fiber_end_detector->set_data(input_image);
            int ret = m_fiber_end_detector->run(); //正常情况下返回 0
            const std::vector<std::vector<st_detect_box>>& result = m_fiber_end_detector->result();
            int count(0);
            for (int j = 0; j < result.size(); j++)
            {
                count += result[j].size();
            }
            //无论是否检测到灰尘，将结果写入到影像并发送到前端
            cv::Mat output_image = draw_boxes_to_image(input_image, result);
            QImage dst_image = convert_cvmat_to_qimage(output_image, 3);
            if (0)
            {
                char sz_path[256] = { 0 };
                sprintf_s(sz_path, "C:/Temp/server_%d.png", i + 1);
                dst_image.save(sz_path);
            }
            st_image_meta meta;
            if (!m_shared_memory_detect_image.write_image(dst_image, meta))
            {
                ret_obj["image"] = "write image error";
            }
            else
            {
                ret_obj["image"] = "success";
                QJsonObject json = image_shared_memory::meta_to_json(meta);
                ret_obj["image_data"] = json;
            }
            emit post_task_finished(QVariant::fromValue(ret_obj));
        }
        result_obj["command"] = "server_anomaly_detection_finish";
        result_obj["param"] = QString::fromStdString("success");
        emit post_task_finished(QVariant::fromValue(result_obj));
    }
    else if(command == "client_request_calibration")
    {
        if (m_auto_focus == nullptr)
        {
            m_auto_focus = new auto_focus(static_cast<void*>(this));
        }
        m_auto_focus->calibrate_model();
        QString current_directory = QCoreApplication::applicationDirPath();
        std::string calibration_file_path = (current_directory + "/calibration.bin").toStdString();
        m_auto_focus->save_model(calibration_file_path);
        result_obj["command"] = "server_calibration_success";
        emit post_task_finished(QVariant::fromValue(result_obj));
    }
    else if(command == "client_request_update_server_parameter")
    {
        QJsonObject param = obj["param"].toObject();
        if (param["name"] == "update_photo_location_list")
        {
            std::vector<st_position> positions;
            QJsonArray posArray = param["photo_location_list"].toArray();
            for (int i = 0; i < posArray.size(); i++)
            {
                QJsonObject obj = posArray[i].toObject();
                int  x = obj["x"].toInt();
                int  y = obj["y"].toInt();
                positions.emplace_back(st_position(x, y));
            }
            if(m_config_data->position_list_changed(positions))
            {
                m_config_data->m_photo_location_list = positions;
                m_config_data->save();
            }
        }
        if (param["name"] == "update_fiber_end_count")
        {
            int fiber_end_count = param["fiber_end_count"].toInt();
            if (m_config_data->m_fiber_end_count != fiber_end_count)
            {
                m_config_data->m_fiber_end_count = fiber_end_count;
                m_config_data->save();
            }
        }
        if (param["name"] == "update_auto_detect")
        {
            int auto_detect = param["auto_detect"].toInt();
            if (m_config_data->m_auto_detect != auto_detect)
            {
                m_config_data->m_auto_detect = auto_detect;
                m_config_data->save();
            }
        }
        if (param["name"] == "update_save_path")
        {
            QString save_path = param["save_path"].toString();
            if (m_config_data->m_save_path != save_path.toStdString())
            {
                m_config_data->m_save_path = save_path.toStdString();
                m_config_data->save();
            }
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
