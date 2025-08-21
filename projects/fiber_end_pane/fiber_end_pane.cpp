#include "fiber_end_pane.h"

#include <QSplitter>


fiber_end_pane::fiber_end_pane()
{

}

bool fiber_end_pane::initialize()
{
    if (!m_is_initialized)
    {
		m_widget = new QWidget();       //插件主界面控件，通常是一个 QWidget 或 QDockWidget

        QVBoxLayout* main_layout = new QVBoxLayout(m_widget);
        main_layout->setContentsMargins(0, 0, 0, 0);
        QSplitter* vertical_splitter = new QSplitter(Qt::Vertical, m_widget);
        main_layout->addWidget(vertical_splitter);
        // 第一行子控件: 影像窗口 + 表格
        QSplitter* top_splitter = new QSplitter(Qt::Horizontal, vertical_splitter);
        m_image_viewer = new image_viewer(top_splitter);
        m_image_viewer->initialize();
        m_status_table = new status_table(top_splitter);
        m_status_table->initialize();
        top_splitter->setStretchFactor(0, 7);
        top_splitter->setStretchFactor(1, 3);
        top_splitter->setCollapsible(0, false);
        top_splitter->setCollapsible(1, false);

        // 第二行子控件: 缩略图控件
        m_thumbnail_bar = new thumbnail_bar(vertical_splitter);
        m_thumbnail_bar->initialize();
        connect(m_thumbnail_bar, &thumbnail_bar::post_thumbnail_clicked, this, &fiber_end_pane::on_thumbnail_clicked);
        vertical_splitter->setStretchFactor(0, 3); // 上部分
        vertical_splitter->setStretchFactor(1, 7); // 下部分
        vertical_splitter->setCollapsible(0, false);
        vertical_splitter->setCollapsible(1, false);

        //使比例设置生效
        QList<int> topSizes;
        topSizes << 700 << 300;
        top_splitter->setSizes(topSizes);
        QList<int> verticalSizes;
        verticalSizes << 200 << 800;
        vertical_splitter->setSizes(verticalSizes);

        m_is_initialized = true;
    }
    return true;
}

void fiber_end_pane::uninitialize()
{
    if (!m_is_initialized)
    {
        return;
    }
    delete m_widget;
    m_widget = nullptr;
    m_is_initialized = false;
}

void fiber_end_pane::set_signal_bridge(signal_bridge* signal_bridge)
{
    m_signal_bridge = signal_bridge;
    if (m_signal_bridge)
    {
        //注册与桥接对象的消息连接
    	connect(m_signal_bridge, &signal_bridge::post_set_fiber_end_pane_image, this, &fiber_end_pane::on_set_fiber_end_pane_image);
    }
}

void fiber_end_pane::on_thumbnail_clicked(int index)
{
	
}

void fiber_end_pane::on_set_fiber_end_pane_image(const QVariant& data)
{
	if(m_image_viewer != nullptr)
	{
		m_image_viewer->set_image((data.value<QImage>()));  // 设置影像数据
	}
}