#include "control_tree_widget.h"

#include <QMessageBox>
#include "control_stacked_widget.h"

control_tree_widget::control_tree_widget(QWidget* parent)
	:QTreeWidget(parent)
{
	
}

control_tree_widget::~control_tree_widget()
{
	
}

void control_tree_widget::initialize()
{
	setHeaderHidden(true);
	m_root_item = new QTreeWidgetItem(this);
	m_root_item->setText(0, QString::fromStdString("参数控制"));
	m_camera_parameter_item = new QTreeWidgetItem(m_root_item);
	m_camera_parameter_item->setText(0, QString::fromStdString("相机控制"));
	m_algorithm_parameter_item = new QTreeWidgetItem(m_root_item);
	m_algorithm_parameter_item->setText(0, QString::fromStdString("算法参数"));
	m_root_item->setExpanded(true);

	//注册消息
	connect(this, &QTreeWidget::itemClicked, this, &control_tree_widget::on_item_clicked);
}

void control_tree_widget::on_item_clicked(QTreeWidgetItem* item, int column)
{
	control_stacked_widget* stacked_widget = dynamic_cast<control_stacked_widget*>(m_stacked_widget);
	if (stacked_widget == nullptr)
	{
		QMessageBox::information(this,
			QString::fromStdString("错误"),
			QString::fromStdString("控件加载失败!"),
			QMessageBox::Ok);
		return;
	}
	if (item == m_camera_parameter_item)
	{
		stacked_widget->setCurrentIndex(0);
	}
	else if (item == m_algorithm_parameter_item)
	{
		stacked_widget->setCurrentIndex(1);
	}
}
