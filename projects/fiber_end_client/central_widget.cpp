#include "central_widget.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QPixmap>

central_widget::central_widget(fiber_end_client* client, QWidget* parent)
	:QWidget(parent),m_client(client)
{
	
}

central_widget::~central_widget()
{
	
}

void central_widget::initialize()
{
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    QSplitter* vertical_splitter = new QSplitter(Qt::Vertical, this);
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
    connect(m_thumbnail_bar, &thumbnail_bar::post_thumbnail_clicked, this, &central_widget::on_thumbnail_clicked);
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
}

void central_widget::set_image(QImage img)
{
	m_image_viewer->set_image(img); // 更新影像显示
}

void central_widget::add_detected_image(QImage img)
{
    m_thumbnail_bar->add_thumbnail(QPixmap::fromImage(img),0);
}


void central_widget::on_thumbnail_clicked(int index)
{
	
}

