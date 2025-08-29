#include "central_thumbnail_bar.h"

#include <QEvent>
thumbnail_bar::thumbnail_bar(QWidget* parent)
    : QWidget(parent)
{

}

thumbnail_bar::~thumbnail_bar()
{

}

void thumbnail_bar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void thumbnail_bar::initialize()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_list_widget = new QListWidget;
    m_list_widget->setViewMode(QListView::IconMode);         // 网格显示
	m_list_widget->setIconSize(QSize(100, 100));        // 缩略图大小
    m_list_widget->setResizeMode(QListWidget::Adjust);       // 窗口改变自动换行
    m_list_widget->setSpacing(5);
    m_list_widget->setMovement(QListView::Static);           // 图标位置固定，不允许拖动重排
    layout->addWidget(m_list_widget);

}

void thumbnail_bar::add_thumbnail(const QPixmap& pixmap, int id)
{
    QListWidgetItem* item = new QListWidgetItem(QIcon(pixmap), "");
    m_list_widget->addItem(item);                               // 自动追加到末尾
}

void thumbnail_bar::clear()
{
    m_thumbnail_list.clear();
    m_selected_ids.clear();
}

void thumbnail_bar::select_thumbnails(const QList<int>& ids)
{
    
}

void thumbnail_bar::on_thumbnail_selected(void* data)
{

}