#pragma once

#include <QtWidgets/QMainWindow>
#include <QStackedWidget>
#include "ui_IndustrialInspection.h"

#include "plugin_manager.h"
#include "../PluginInterface/signal_bridge.h"

class IndustrialInspection : public QMainWindow
{
    Q_OBJECT

public:
    IndustrialInspection(QWidget *parent = nullptr);
    virtual ~IndustrialInspection() override;

    void closeEvent(QCloseEvent* event) override;

	void load_plugins();        // 加载所有插件

    void initialize_camera_view();    //左侧相机树插件

    void initialize_control_pane();   //右侧控制面板

    void initialize_fiber_end_pane();   //中心控制面板，端面检测

signals:
    void post_set_control_pane_camera(const QVariant& data);            //向控制面板发送消息，通知更新界面
    void post_set_fiber_end_pane_image(const QVariant& data);            //向端面检测面板发送消息，通知更新界面
private slots:
    void on_camera_opened(const QVariant& data);    // 槽函数,接收 camera_view  插件发送的打开相机消息
	void on_trigger_finished(const QVariant& data); // 槽函数,接收 control_pane 插件发送的采图完成消息
private:
    Ui::IndustrialInspectionClass ui;
    plugin_manager m_plugin_manager;
	QStackedWidget* m_stacked_widget{nullptr};          // 用于管理不同插件的界面
    signal_bridge* m_signal_bridge{ nullptr };          // 唯一桥接对象，插件与主程序之间通过信号槽通信，该对象负责中转
    QMap<QString, interface_plugin*> m_loaded_plugins;            // 存储所有已初始化的插件，用于动态加载时避免重复加载，不负责资源释放
    QList<QDockWidget*> m_dock_widgets;                 // 存储所有的 QDockWidget 插件,主界面退出时需要关闭它们
    QList<QWidget*> m_widgets;                          // 存储所有的 QWidget 插件,主界面退出时需要关闭它们
};

