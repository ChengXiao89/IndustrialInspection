#pragma once

#include <QTableWidget>
#include <QTableWidgetItem>

#include "../device_enum/device_info.hpp"

class camera_info_widget : public QTableWidget
{
	Q_OBJECT
public:
	camera_info_widget(QWidget* parent = nullptr);
	~camera_info_widget();

	void initialize();

	void set_device_info(const st_device_info* device_info);
	//st_device_info* device_info() const { return m_device_info; }

	void update_device_info(const st_device_info* device_info);

private:
	st_device_info* m_device_info{ nullptr };

};