#include "thumbnail_bar.h"

#include <QEvent>
thumbnail_bar::thumbnail_bar(QWidget* parent)
    : QWidget(parent)
{
   
}

thumbnail_bar::~thumbnail_bar()
{
	
}

bool thumbnail_bar::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) 
    {
        QLabel* label = qobject_cast<QLabel*>(watched);
        if (label) 
        {
            int id = label->property("thumb_id").toInt();
            emit post_thumbnail_clicked(id);
            // 设置点击后的单选高亮
            select_thumbnails({ id });
        }
    }
    return QWidget::eventFilter(watched, event);
}

void thumbnail_bar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update_grid_layout();
}

void thumbnail_bar::initialize()
{
    setMinimumSize(480, 360);
    m_scroll_area = new QScrollArea(this);
    m_scroll_area->setWidgetResizable(true);
    m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_container = new QWidget;
    m_grid_layout = new QGridLayout(m_container);
    m_grid_layout->setSpacing(8);
    m_grid_layout->setContentsMargins(4, 4, 4, 4);
    m_scroll_area->setWidget(m_container);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_scroll_area);

}

void thumbnail_bar::add_thumbnail(const QPixmap& pixmap, int id)
{
    QLabel* label = new QLabel;
    label->setFixedSize(m_thumbnail_size);
    label->setPixmap(pixmap.scaled(m_thumbnail_size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setAlignment(Qt::AlignCenter);
    label->setFrameStyle(QFrame::StyledPanel);
    label->setCursor(Qt::PointingHandCursor);
    label->setProperty("thumb_id", id);
    label->installEventFilter(this);

    m_thumbnail_list.append({ id, label });
    update_grid_layout();
}

void thumbnail_bar::clear()
{
    for (auto& thumb : m_thumbnail_list) 
    {
        m_grid_layout->removeWidget(thumb.label);
        delete thumb.label;
    }
    m_thumbnail_list.clear();
    m_selected_ids.clear();
}

void thumbnail_bar::select_thumbnails(const QList<int>& ids)
{
    m_selected_ids = ids;
    for (auto& thumb : m_thumbnail_list) 
    {
        bool is_selected = false;
        for (int value : m_selected_ids)
        {
            if(value == thumb.m_id)
            {
                is_selected = true;
				break; // 找到后跳出循环
            }
        }
        if(is_selected)
        {
            thumb.label->setStyleSheet("border: 2px solid red;"); // 被选中的
        }
        else 
        {
            thumb.label->setStyleSheet(""); // 清除样式
        }
    }
}

void thumbnail_bar::update_grid_layout()
{
    int width = m_scroll_area->viewport()->width();
    int spacing = m_grid_layout->spacing();
    int margin = m_grid_layout->contentsMargins().left() + m_grid_layout->contentsMargins().right();
    int total = width - margin;
    int cell_width = m_thumbnail_size.width() + spacing;
    int columns = qMax(1, total / cell_width);
    if (columns == m_columns && !m_grid_layout->isEmpty())
    {
        return;
    }
    m_columns = columns;

    // 清空布局
    QLayoutItem* item;
    while ((item = m_grid_layout->takeAt(0)) != nullptr)
    {
	    continue;   // 不删除控件，只移除布局
    }
    // 重新布局
    for (int i = 0; i < m_thumbnail_list.size(); ++i) 
    {
        int row = i / columns;
        int col = i % columns;
        m_grid_layout->addWidget(m_thumbnail_list[i].label, row, col);
    }
    // 恢复当前高亮状态
    select_thumbnails(m_selected_ids);
}

void thumbnail_bar::on_thumbnail_selected(void* data)
{
	
}