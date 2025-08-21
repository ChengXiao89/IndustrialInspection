/*****************************************
 * 信号桥接对象，连接并转发所有插件的信号
 * 本质上仍然是   插件--->主程序(中转)--->插件 的通信机制
 * 该类对象由主程序维护，全局唯一
 ******************************************/
#pragma once

#include <QObject>
#include <QVariant>

#include "plugininterface_global.h"

class PLUGININTERFACE_EXPORT signal_bridge : public QObject
{
    Q_OBJECT
public:
    explicit signal_bridge(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void post_camera_opened(const QVariant& data);              //转发 camera_view 插件的打开相机信号，通知主程序加载 control_pane 插件并初始化
    void post_set_control_pane_camera(const QVariant& data);    //转发主程序设置相机信号，通知 control_pane 插件获取相机数据并设置界面内容
	void post_trigger_finished(const QVariant& data);           //转发 control_pane 插件的采图完成信号，通知主程序更新界面或进行后续处理
    void post_set_fiber_end_pane_image(const QVariant& data);   //转发主程序显示影像信号，通知 fiber_end_pane 插件显示影像
};