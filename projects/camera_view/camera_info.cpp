#include "camera_info.h"
#include <QHeaderView>

camera_info_widget::camera_info_widget(QWidget* parent)
	: QTableWidget(parent)
{
	
}

camera_info_widget::~camera_info_widget()
{
	// 清理资源
	setRowCount(0);
	m_device_info = nullptr;
}

void camera_info_widget::initialize()
{
	setColumnCount(2);
	verticalHeader()->setVisible(false);
	horizontalHeader()->setVisible(false);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
}

void camera_info_widget::set_device_info(const st_device_info* device_info)
{
	if(m_device_info == device_info)
	{
		return;
	}
	m_device_info = const_cast<st_device_info*>(device_info); //设置设备信息
	setRowCount(0);
	if (m_device_info != nullptr)
	{
		for (const auto& item : m_device_info->m_cam_items)
		{
			int row = rowCount();
			insertRow(row);
			setItem(row, 0, new QTableWidgetItem(item.m_key));
			setItem(row, 1, new QTableWidgetItem(item.m_value));
		}
	}
}

void camera_info_widget::update_device_info(const st_device_info* device_info)
{
	if (m_device_info != device_info)
	{
		return;
	}
	setRowCount(0);
	if (m_device_info != nullptr)
	{
		for (const auto& item : m_device_info->m_cam_items)
		{
			int row = rowCount();
			insertRow(row);
			setItem(row, 0, new QTableWidgetItem(item.m_key));
			setItem(row, 1, new QTableWidgetItem(item.m_value));
		}
	}
}
