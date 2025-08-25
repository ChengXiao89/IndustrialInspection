#pragma once
#include <QTreeWidget>

class control_tree_widget : public QTreeWidget
{
	Q_OBJECT
public:
	control_tree_widget(QWidget* parent = nullptr);
	~control_tree_widget();

	void initialize();		//初始化

	//设置关联控件,点击树控件上某一节点时关联控件显示相应的页面
	void set_stacked_widget(QWidget* stacked_widget) { m_stacked_widget = stacked_widget; }

private slots:
	void on_item_clicked(QTreeWidgetItem* item, int column);
private:
	QWidget* m_stacked_widget{ nullptr };
	QTreeWidgetItem* m_root_item{ nullptr };
	QTreeWidgetItem* m_camera_parameter_item{ nullptr };
	QTreeWidgetItem* m_algorithm_parameter_item{ nullptr };
	QTreeWidgetItem* m_main_operate_parameter_item{ nullptr };
};