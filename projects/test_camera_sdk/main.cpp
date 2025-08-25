#include <QtCore/QCoreApplication>

#include <vector>
#include <iostream>

#include "../device_enum/device_enum_factory.h"
#include "../device_camera/camera_factory.h"




int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    /***********************1.枚举所有已经连接的设备*************************/
    TYPE_SDK sdk_type{ SDK_DVP2 };      //使用度申的SDK
    interface_device_enum* device_enum = device_enum_factory::create_device_enum(sdk_type);
    std::vector<st_device_info*> device_info_list = device_enum->enumerate_devices();
    //打印相机信息
    for (int i = 0;i < device_info_list.size();i++)
    {
        st_device_info* device_info = device_info_list[i];
        if(device_info == nullptr)
        {
	        continue;
        }
        std::cout << "*********************************************** " << std::endl;
        std::cout << "camera_id : "<< device_info->m_unique_id.toStdString() << std::endl;
        for (int j = 0;j < device_info->m_cam_items.size();j++)
        {
            std::cout << std::string(device_info->m_cam_items[j].m_key.toLocal8Bit()) << " : "<< std::string(device_info->m_cam_items[j].m_value.toLocal8Bit()) << std::endl;
        }
    }
    //打开第一个相机
    interface_camera* camera = camera_factory::create_camera(device_info_list[0]);
    if(camera == nullptr)
    {
        std::cout << "create camera fail!" << std::endl;
	    return 0;
    }
    if(camera->open() != STATUS_SUCCESS)
    {
        std::cout << "open camera fail!" << std::endl;
        return 0;
    }
    //参数设置接口查看interface_camera头文件.包括曝光、增益、ROI等
    //...


    //这里设置为触发模式-软触发
    camera->set_trigger_mode(global_trigger_mode_once);
    camera->set_trigger_source(global_trigger_source_software);
    //拍照。 分为两个步骤:(1) 开始采集 （2）拍照
    if(camera->start_grab() != STATUS_SUCCESS)
    {
        std::cout << "start_grab fail!" << std::endl;
        return 0;
    }
    
    QImage img = camera->trigger_once();        //触发一次，拍照取图. 在调用了 start_grab 之后可以反复拍照
    //测试保存到文件
    img.save("D:/1.jpg");

    //停止采集
    camera->stop_grab();
    //关闭相机
    camera->close();

    //释放资源
    for (int i = 0;i < device_info_list.size();i++)
    {
        delete device_info_list[i];
    }
    device_info_list.clear();
    delete camera;
    std::cout << "finish" << std::endl;
    return 0;
}
