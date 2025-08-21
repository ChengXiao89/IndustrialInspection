#pragma once
#include "interface.h"

class PLUGININTERFACE_EXPORT interface_camera_view : public interface_plugin
{
public:
    interface_camera_view() = default;
    virtual ~interface_camera_view() = default;

    // 打开相机
    virtual void open_camera(const QString& camera_id) = 0;

    // 设置相机参数
    virtual void set_camera_parameter(const QString& camera_id) = 0;

    
};

#define InterfaceCamera_iid "com.mycompany.InterfaceCamera"
Q_DECLARE_INTERFACE(interface_camera_view, InterfaceCamera_iid)