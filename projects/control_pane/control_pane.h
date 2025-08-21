#pragma once

#include "control_pane_global.h"
#include "../PluginInterface/interface_control_pane.h"
#include "../PluginInterface/interface_ui.h"

#include <QDockWidget>
#include "control_tree_widget.h"
#include "control_stacked_widget.h"

/**************************************
 * 插件类定义
 * Q_PLUGIN_METADATA(IID ...) 必须与某个 Q_DECLARE_INTERFACE 中注册的一致；插件类必须是 QObject 的子类并且有 Q_OBJECT 宏
 * 如果插件类继承了多个接口（如 IPluginInterface、IPluginInterface_UI、inference_control_pane，
 * Q_PLUGIN_METADATA(IID "...") 中的 IID 应该与你希望通过 qobject_cast 或 QPluginLoader::instance()->qt_metacast(...)
 * 动态识别的主接口保持一致，多个插件都使用相同的 IID 也没问题
 * IID 的作用是用来让宿主程序识别接口类型，而不是用来区分不同插件
 ***************************************/

class CONTROL_PANE_EXPORT control_pane : public QObject, public inference_control_pane, public interface_plugin_ui
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID interface_plugin_iid)
	Q_INTERFACES(interface_plugin interface_plugin_ui inference_control_pane)
public:
    control_pane();

    // IPluginInterface 接口
    virtual QString name() const override { return QString("ControlPanePlugin"); } //插件名称需要唯一，否则会因为名称冲突导致无法加载
    virtual QString version() const override { return QString("v1.0"); }
    virtual QString description() const override { return QString("show control ui and camera parameters"); }
    virtual QString author() const override { return QString("Fuguang Technology Co., Ltd"); }
    virtual bool initialize() override;         //初始化控件
    virtual void uninitialize() override;
    virtual bool is_initialized() const override { return m_is_initialized; }
    virtual bool enable() override { return m_is_enabled; }
    virtual void disable()  override {}
    virtual bool is_enabled() const  override { return m_is_enabled; }
    virtual QString get_configuration() const override { return QString(""); }
    virtual bool set_configuration(const QString& config) override { return true; }
    virtual QString get_last_error() const override { return QString(""); }
    virtual bool execute() override { return true; }
    //virtual void register_event_bus(event_bus* event_bus) override { m_event_bus = event_bus; }
    virtual void set_signal_bridge(signal_bridge* signal_bridge) override;      //桥接对象


    // interface_plugin_ui 接口
    QWidget* widget() const override { return m_dock_widget; }
    UIType widget_type() const override { return interface_plugin_ui::UIType::DockWidget; }


public slots:
    //设置相机并在界面上显示相机相关的参数以及状态
	void on_set_camera(const QVariant& data);
	//采图完成后，向主程序发送消息
    void on_trigger_finished(const QVariant& data);

signals:
	void post_trigger_finished(const QVariant& data);              //转发子控件发送的采图完毕消息

private:
	bool m_is_initialized{ false };         //是否已经初始化
	bool m_is_enabled{ true };              //是否启用,目前将插件设置为始终启用，资源随主程序关闭而释放
    signal_bridge* m_signal_bridge{ nullptr };  //桥接对象，通过该对象向主程序发消息
	QDockWidget* m_dock_widget{ nullptr };
	control_tree_widget* m_control_tree_widget{ nullptr };
	control_stacked_widget* m_control_stacked_widget{ nullptr };

};
