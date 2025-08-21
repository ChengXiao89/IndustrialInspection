/**********************************
 * 端面检测特有插件. 显示影像数据、检测结果和提供交互功能
 * 包含三个子插件
 * (1) 第一行左侧，影像浏览窗口,查看影像数据
 * (2) 第一行右侧, 表格控件，显示检测结果，每一行代表一张影像的检测结果
 * (3) 第二行缩略图控件，显示每个检测结果的缩略图
 **********************************/
#pragma once

#include "fiber_end_pane_global.h"
#include "../PluginInterface/interface.h"
#include "../PluginInterface/interface_ui.h"

#include "image_viewer.h"
#include "status_table.h"
#include "thumbnail_bar.h"

class FIBER_END_PANE_EXPORT fiber_end_pane : public QObject, public interface_plugin, public interface_plugin_ui
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID interface_plugin_iid)
    Q_INTERFACES(interface_plugin interface_plugin_ui)
public:
    fiber_end_pane();

    // IPluginInterface 接口
    virtual QString name() const override { return QString("FiberEndPanePlugin"); } //插件名称需要唯一，否则会因为名称冲突导致无法加载
    virtual QString version() const override { return QString("v1.0"); }
    virtual QString description() const override { return QString("fiber-end detection control ui"); }
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
    QWidget* widget() const override { return m_widget; }
    UIType widget_type() const override { return interface_plugin_ui::UIType::Widget; }

private slots:
    void on_thumbnail_clicked(int index);
	void on_set_fiber_end_pane_image(const QVariant& data);         //界面触发一次拍照之后，主程序转发的消息，用于显示拍照的影像
private:
    bool m_is_initialized{ false };         //是否已经初始化
    bool m_is_enabled{ true };              //是否启用,目前将插件设置为始终启用，资源随主程序关闭而释放
    signal_bridge* m_signal_bridge{ nullptr };  //桥接对象，通过该对象向主程序发消息
    QWidget* m_widget{ nullptr };
    image_viewer* m_image_viewer{ nullptr };
    status_table* m_status_table{ nullptr };
    thumbnail_bar* m_thumbnail_bar{ nullptr };
};