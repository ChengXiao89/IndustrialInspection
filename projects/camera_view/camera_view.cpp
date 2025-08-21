#include "camera_view.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QMessageBox>

camera_view::camera_view()
{
    
}

bool camera_view::initialize()
{
    if (!m_is_initialized)
    {
        m_dock_widget = new QDockWidget("相机列表");
        QWidget* inner = new QWidget(m_dock_widget);
        QSplitter* splitter = new QSplitter(Qt::Vertical, inner);
        m_camera_tree_widget = new camera_tree_widget(splitter);
        m_camera_tree_widget->initialize ();
        connect(m_camera_tree_widget, &camera_tree_widget::post_camera_opened, this, &camera_view::on_camera_opened);

        m_camera_info_widget = new camera_info_widget(splitter);
        m_camera_info_widget->initialize();
		m_camera_tree_widget->set_info_widget(m_camera_info_widget); //设置树控件的相机信息控件
        QVBoxLayout* layout = new QVBoxLayout(inner);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(splitter);
        m_dock_widget->setWidget(inner);
        m_dock_widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);

        m_is_initialized = true;
    }
    return true;
}

void camera_view::uninitialize()
{
    if (!m_is_initialized)
    {
        return;
    }
    delete m_dock_widget;
    m_dock_widget = nullptr;

    m_is_initialized = false;
}

void camera_view::set_signal_bridge(signal_bridge* signal_bridge)
{
    m_signal_bridge = signal_bridge;
    if (m_signal_bridge)
    {
        connect(this, &camera_view::post_camera_opened, m_signal_bridge, &signal_bridge::post_camera_opened);
    }
}

void camera_view::on_camera_opened(const QVariant& data)
{
    emit post_camera_opened(data);
}