#include "control_pane.h"
#include <QSplitter>
#include <QBoxLayout>

control_pane::control_pane()
{

}

bool control_pane::initialize()
{
    if (!m_is_initialized)
    {
        m_dock_widget = new QDockWidget("控制面板");
        QWidget* inner = new QWidget(m_dock_widget);
        QSplitter* splitter = new QSplitter(Qt::Horizontal, inner);
        m_control_tree_widget = new control_tree_widget(splitter);
        m_control_tree_widget->initialize();
        m_control_stacked_widget = new control_stacked_widget(splitter);
        m_control_stacked_widget->initialize();
		//m_control_stacked_widget 包含两个子窗口，第一个子窗口是相机参数窗口，第二个子窗口是算法参数窗口
		camera_parameter_widget* widget = dynamic_cast<camera_parameter_widget*>(m_control_stacked_widget->widget(0));
        connect(widget, &camera_parameter_widget::post_trigger_finished, this, &control_pane::on_trigger_finished);
        m_control_tree_widget->set_stacked_widget(m_control_stacked_widget); //设置树控件的相机信息控件
        //设置左右控件的宽度显示比例
        QList<int> sizes;
        sizes << 5;  // 树控件: 5/12
        sizes << 7;  // 堆栈控件: 7/12
        splitter->setSizes(sizes);

        QVBoxLayout* layout = new QVBoxLayout(inner);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(splitter);
        m_dock_widget->setWidget(inner);
        m_dock_widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);

        m_is_initialized = true;
    }
    return true;
}

void control_pane::uninitialize()
{
    if (!m_is_initialized)
    {
        return;
    }
    delete m_dock_widget;
    m_dock_widget = nullptr;
    m_is_initialized = false;
}

void control_pane::set_signal_bridge(signal_bridge* signal_bridge)
{
    m_signal_bridge = signal_bridge;
    if (m_signal_bridge)
    {
        //注册与桥接对象的消息连接
        connect(m_signal_bridge, &signal_bridge::post_set_control_pane_camera, this, &control_pane::on_set_camera);
        connect(this, &control_pane::post_trigger_finished, m_signal_bridge, &signal_bridge::post_trigger_finished);
    }
}

void control_pane::on_set_camera(const QVariant& data)
{
    m_control_stacked_widget->set_camera(data);
}

void control_pane::on_trigger_finished(const QVariant& data)
{
    emit post_trigger_finished(data);
}
