#include "IndustrialInspection.h"

#include <QDir>
#include <QMessageBox>


#include "../camera_view/camera_view.h"
#include "../control_pane/control_pane.h"
#include "../fiber_end_pane/fiber_end_pane.h"

IndustrialInspection::IndustrialInspection(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    resize(1280, 720);
    m_signal_bridge = new signal_bridge(this);
	m_stacked_widget = new QStackedWidget(this);
	setCentralWidget(m_stacked_widget); // 设置中心控件为 QStackedWidget
    load_plugins();
    initialize_camera_view();
    initialize_fiber_end_pane();      //中央插件需要先初始化，否则在动态加载时会导致窗口闪烁
	//m_stacked_widget->hide();           // 初始时隐藏中心控件
}

IndustrialInspection::~IndustrialInspection()
{}

void IndustrialInspection::load_plugins()
{
    QDir pluginDir(qApp->applicationDirPath() + "/plugins");
	m_plugin_manager.load_plugins(pluginDir.absolutePath());
	m_plugin_manager.initialize_plugins();
}

void IndustrialInspection::initialize_camera_view()
{
    for (interface_plugin* base : m_plugin_manager.get_plugins())
    {
        if (base->name() != QString::fromStdString("CameraViewPlugin"))
        {
	        continue;
        }
        camera_view* camera_view_plugin = dynamic_cast<camera_view*>(base);
        if (camera_view_plugin != nullptr)
        {
            m_loaded_plugins.insert(base->name(), base);
            // 获取插件中的主界面控件（通常是 QDockWidget）
            QWidget* plugin_widget = camera_view_plugin->widget();
            if (auto* dock = qobject_cast<QDockWidget*>(plugin_widget))
            {
                dock->setMinimumWidth(180);
                dock->setMaximumWidth(360);
                addDockWidget(Qt::LeftDockWidgetArea, dock);
                m_dock_widgets.append(dock);  // 存储到列表中以便在关闭时处理
            }
            else
            {
                // 否则作为普通窗口或 tab 添加
                m_stacked_widget->addWidget(plugin_widget);
                m_stacked_widget->setCurrentWidget(nullptr);
                m_widgets.append(plugin_widget);  // 存储到列表中以便在关闭时处理
            }
            /*******************关联信号槽********************/
            camera_view_plugin->set_signal_bridge(m_signal_bridge);
            //信号路径: 相机树控件-->相机视图插件-->桥接对象-->主程序，主程序打开控制面板插件之后向其发送消息
            connect(m_signal_bridge, &signal_bridge::post_camera_opened, this, &IndustrialInspection::on_camera_opened);
        }
    }

}

void IndustrialInspection::initialize_control_pane()
{
    for (interface_plugin* base : m_plugin_manager.get_plugins())
    {
        if (base->name() != QString::fromStdString("ControlPanePlugin"))
        {
            continue;
        }
        control_pane* control_pane_plugin = dynamic_cast<control_pane*>(base);
        if (control_pane_plugin != nullptr)
        {
            m_loaded_plugins.insert(base->name(), base);
            // 获取插件中的主界面控件（通常是 QDockWidget）
            QWidget* plugin_widget = control_pane_plugin->widget();
            if (auto* dock = qobject_cast<QDockWidget*>(plugin_widget))
            {
                dock->setMinimumWidth(360);
                dock->setMaximumWidth(480);
                addDockWidget(Qt::RightDockWidgetArea, dock);
                m_dock_widgets.append(dock);  // 存储到列表中以便在关闭时处理
            }
            else
            {
                // 否则作为普通窗口或 tab 添加
                m_stacked_widget->addWidget(plugin_widget);
                m_stacked_widget->setCurrentWidget(nullptr);
                m_widgets.append(plugin_widget);  // 存储到列表中以便在关闭时处理
            }
            /*******************关联信号槽********************/
            control_pane_plugin->set_signal_bridge(m_signal_bridge);
			//主程序打开相机后，通过桥接对象向控制面板插件发送消息，设置相机数据
            connect(this, &IndustrialInspection::post_set_control_pane_camera, m_signal_bridge, &signal_bridge::post_set_control_pane_camera);
            //主程序接收桥接对象转发的取图消息，打开端面检测插件并设置影像数据
            connect(m_signal_bridge, &signal_bridge::post_trigger_finished, this, &IndustrialInspection::on_trigger_finished);
        }
    }


}

void IndustrialInspection::initialize_fiber_end_pane()
{
    for (interface_plugin* base : m_plugin_manager.get_plugins())
    {
        if (base->name() != QString::fromStdString("FiberEndPanePlugin"))
        {
            continue;
        }
        fiber_end_pane* fiber_end_pane_plugin = dynamic_cast<fiber_end_pane*>(base);
        if (fiber_end_pane_plugin != nullptr)
        {
            m_loaded_plugins.insert(base->name(), base);
            // 端面检测插件，为中心控件
            QWidget* plugin_widget = fiber_end_pane_plugin->widget();
            if (auto* dock = qobject_cast<QDockWidget*>(plugin_widget))
            {
                dock->setMinimumWidth(360);
                dock->setMaximumWidth(480);
                addDockWidget(Qt::RightDockWidgetArea, dock);
                m_dock_widgets.append(dock);  // 存储到列表中以便在关闭时处理
            }
            else
            {
                // 否则作为普通窗口或 tab 添加
                //setCentralWidget(plugin_widget);  // 仅示例
                m_stacked_widget->addWidget(plugin_widget);
                m_stacked_widget->setCurrentWidget(nullptr);
            	m_widgets.append(plugin_widget);  // 存储到列表中以便在关闭时处理
            }
            /*******************关联信号槽********************/
            fiber_end_pane_plugin->set_signal_bridge(m_signal_bridge);
            connect(this, &IndustrialInspection::post_set_fiber_end_pane_image, m_signal_bridge, &signal_bridge::post_set_fiber_end_pane_image);

        }
    }

}

void IndustrialInspection::on_camera_opened(const QVariant& data)
{
    QMap<QString, interface_plugin*>::iterator  iter = m_loaded_plugins.find("ControlPanePlugin");
    if(iter == m_loaded_plugins.end())
    {
        initialize_control_pane();
        iter = m_loaded_plugins.find("ControlPanePlugin");
        if (iter == m_loaded_plugins.end())
        {
            QMessageBox::information(this, QString::fromStdString("错误"), 
                QString::fromStdString("加载控制面板插件失败!"), QMessageBox::Ok);
            return;
        }
    }
    control_pane* control_pane_plugin = dynamic_cast<control_pane*>(iter.value());
    if(control_pane_plugin == nullptr)
    {
        QMessageBox::information(this, QString::fromStdString("错误"),
            QString::fromStdString("获取控制面板插件失败!"), QMessageBox::Ok);
        return;
    }
    interface_camera* lp_camera = static_cast<interface_camera*>(data.value<void*>());
    if (lp_camera != nullptr)
    {
        control_pane_plugin->widget()->show();
    }
    else
    {
        control_pane_plugin->widget()->close();
    }
    //无论是关闭还是打开相机，都要关闭端面检测插件
    if(m_stacked_widget->isVisible())
    {
		//m_stacked_widget->hide(); //隐藏中心控件
    }
    emit post_set_control_pane_camera(data);
}

void IndustrialInspection::on_trigger_finished(const QVariant& data)
{
    QMap<QString, interface_plugin*>::iterator iter = m_loaded_plugins.find("FiberEndPanePlugin");
    if (iter == m_loaded_plugins.end())
    {
        initialize_fiber_end_pane();
        iter = m_loaded_plugins.find("FiberEndPanePlugin");
        if (iter == m_loaded_plugins.end())
        {
            QMessageBox::information(this, QString::fromStdString("错误"),
                QString::fromStdString("加载端面检测插件失败!"), QMessageBox::Ok);
            return;
        }
    }
	if(!m_stacked_widget->isVisible())
	{
		m_stacked_widget->show(); // 显示中心控件
	}
    //显示影像
    emit post_set_fiber_end_pane_image(data);
}

void IndustrialInspection::closeEvent(QCloseEvent* event)
{
    /*************1.关闭移除所有插件************/
    for (QDockWidget* dock_widget : m_dock_widgets) 
    {
        removeDockWidget(dock_widget);
        //delete dock;
    }
    m_dock_widgets.clear();
	m_stacked_widget->setCurrentWidget(nullptr); // 清除当前显示的插件
    for (QWidget* widget : m_widgets)
    {
        m_stacked_widget->removeWidget(widget);
    }
    m_widgets.clear();
    /***************2.卸载所有插件**************/
    m_plugin_manager.unload_plugins();
    QMainWindow::closeEvent(event);
    qApp->quit();  // 确保退出
}