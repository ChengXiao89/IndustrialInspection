#pragma once
#include <QWidget>

#include "central_image_viewer.h"
#include "central_status_table.h"
#include "central_thumbnail_bar.h"

#include "client.h"

class central_widget : public QWidget
{
	Q_OBJECT
public:
	central_widget(fiber_end_client* client, QWidget* parent = nullptr);
	~central_widget() override;

	void initialize();	// 初始化中心窗口视图

	void set_image(QImage img);	// 更新影像显示

private slots:
	void on_thumbnail_clicked(int index);

private:
	fiber_end_client* m_client{ nullptr }; // 客户端对象，用于与服务器通信
	image_viewer* m_image_viewer{ nullptr };
	status_table* m_status_table{ nullptr };
	thumbnail_bar* m_thumbnail_bar{ nullptr };
};
