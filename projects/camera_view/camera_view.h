#pragma once
#include <QObject>
#include <QDockWidget>
#include <QTreeWidget>
#include "../PluginInterface/interface_camera_view.h"
#include "../PluginInterface/interface_ui.h"

#include "camera_view_global.h"
#include "camera_tree.h"
#include "camera_info.h"


class CAMERA_VIEW_EXPORT camera_view : public QObject, public interface_camera_view, public interface_plugin_ui
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID interface_plugin_iid)
	Q_INTERFACES(interface_plugin interface_plugin_ui interface_camera_view)
public:
    camera_view();

    // interface_plugin 接口
    virtual QString name() const override   { return QString("CameraViewPlugin"); }//插件名称需要唯一，否则会因为名称冲突导致无法加载
    virtual QString version() const override { return QString("v1.0"); }
    virtual QString description() const override { return QString("show available cameras"); }
    virtual QString author() const override { return QString("Fuguang Technology Co., Ltd"); }
    virtual bool initialize() override;         //初始化控件以及相机 SDK 
    virtual void uninitialize() override; 
    virtual bool is_initialized() const override{ return m_is_initialized; }
    virtual bool enable() override { return m_is_enabled; }
    virtual void disable()  override{}
    virtual bool is_enabled() const  override { return m_is_enabled; }
    virtual QString get_configuration() const override { return QString(""); }
    virtual bool set_configuration(const QString& config) override { return true; }
    virtual QString get_last_error() const override { return QString(""); }
    virtual bool execute() override { return true; }
    //virtual void register_event_bus(event_bus* event_bus) override { m_event_bus = event_bus; }   //消息总线
    virtual void set_signal_bridge(signal_bridge* signal_bridge) override;      //桥接对象

    // interface_camera_view 接口
    virtual void open_camera(const QString& camera_id) override {}
    virtual void set_camera_parameter(const QString& camera_id) override{}
    

    // interface_plugin_ui 接口
    virtual QWidget* widget() const override { return m_dock_widget; }
    virtual UIType widget_type() const override { return interface_plugin_ui::UIType::DockWidget; }

private slots:
    void on_camera_opened(const QVariant& data);                //响应子控件发送的打开相机消息，进行转发

signals:
	void post_camera_opened(const QVariant& data);              //转发子控件发送的打开相机消息

private:
	bool m_is_initialized{ false };         //是否已经初始化
	bool m_is_enabled{ true };              //是否启用,目前将插件设置为始终启用，资源随主程序关闭而释放
    signal_bridge* m_signal_bridge{ nullptr };  //桥接对象，通过该对象向主程序发消息
	QDockWidget* m_dock_widget{ nullptr };
    camera_tree_widget* m_camera_tree_widget{ nullptr };
	camera_info_widget* m_camera_info_widget{ nullptr };
};
