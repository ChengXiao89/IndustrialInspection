#pragma once

#include <QIcon>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QResizeEvent>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "client.h"
#include "../device_enum/device_info.hpp"

struct st_camera_info
{
	QString m_unique_id;				//相机唯一标识符
	std::vector<st_cam_item> m_items;	//相机参数项列表

};

class camera_tree_widget : public QTreeWidget
{
	Q_OBJECT
public:
	camera_tree_widget(QWidget* parent = nullptr);
	virtual ~camera_tree_widget() override;

	virtual void resizeEvent(QResizeEvent* event) override;

	//初始化，添加根节点，注册消息，检测所有连接的相机设备,初始化右键菜单
	void initialize();

	void set_info_widget(QWidget* widget = nullptr) { m_widget = widget; }

	// 打开/关闭相机之后,更新所有节点图标,刷新树控件时也需要更新所有节点图标
	void update_item_icon(const QString& unique_id);

	//接受服务端发送过来的消息，更新相机列表
	void update_camera_list(const QJsonObject& obj);
	//更新相机列表时调用，更新 m_device_list
	void update_device_list(std::vector<st_camera_info*>& new_device_list);
	//更新相机列表时调用，更新 m_device_list 之后，更新树控件
	void update_tree();
	//查找树控件中是否存在对应节点
	bool is_unique_id_in_tree(const QString& unique_id) const;
	void add_camera_item(const st_camera_info* cam_info);

	static std::vector<st_camera_info*> json_to_device_list(const QJsonObject& root); //将 JSON 对象转换为设备列表
private slots:
	void on_item_clicked(QTreeWidgetItem* item, int column);

signals:
	void post_request_update_camera_list();						//更新相机列表，向后端发送消息
	void post_request_open_camera(const QVariant& data);		//打开相机，向后端发送消息
	void post_request_close_camera();							//关闭相机，向后端发送消息

private:
	QMap<QString,st_camera_info*> m_device_list;			 // 设备列表，存储所有连接的设备信息
	QTreeWidgetItem* m_root_item{ nullptr };                 // 树控件根节点，管理所有相机节点
	QWidget* m_widget{ nullptr };							 // 关联控件，相机信息显示控件
	QString m_current_unique_id{ "" };					 // 当前打开的相机唯一标识符
	QIcon m_icon_opened;		//表示设备处于打开状态
	QIcon m_icon_closed;		//表示设备处于关闭状态
	QIcon m_icon_disabled;		//表示设备处于不可用状态(被其他程序打开)
	QIcon m_icon_refresh;		//刷新连接设备及其状态
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class camera_info_widget : public QTableWidget
{
	Q_OBJECT
public:
	camera_info_widget(QWidget* parent = nullptr);
	~camera_info_widget();

	void initialize();

	void update_camera_info(const st_camera_info* cam_info);
	QString m_unique_id{ "" };		//相机唯一标识符
private:
};

///////////////////////////////////////////////////////////////////////////////////////////////
class camera_view_dock_widget : public QDockWidget
{
	Q_OBJECT
public:
	camera_view_dock_widget(fiber_end_client* client, QWidget* parent = nullptr);
	virtual ~camera_view_dock_widget() override;
	void initialize();				// 初始化相机视图


private slots:
	void on_update_camera_status(const QVariant& data);	// 接收后端的相机信息，包括相机列表+打开的相机，仅在建立连接之后调用，后端-->界面
	void on_update_camera_list(const QVariant& data);	// 接收后端的设备列表数据，更新数据。后端-->界面
	void on_camera_opened_success(const QVariant& data);// 相机打开成功，更新相机信息界面和图标状态，后端-->界面
	void on_camera_closed_success();					// 相机关闭成功，更新相机信息界面和图标状态，后端-->界面

	void on_request_update_camera_list();				// 界面刷新相机列表，界面-->后端
	void on_request_open_camera(const QVariant& data);	// 界面打开相机，界面-->后端
	void on_request_close_camera();						// 界面关闭相机，界面-->后端	
signals:
	void post_camera_opened_success(const QJsonObject& data);	//打开相机之后，向主界面发送消息，显示控制面板并更新相机参数
	void post_camera_closed_success();							//关闭相机之后，向主界面发送消息，隐藏控制面板并清空相机参数
private:
	fiber_end_client* m_client{ nullptr };  // 客户端对象，用于与后端通信
	camera_tree_widget* m_camera_tree_widget{ nullptr };
	camera_info_widget* m_camera_info_widget{ nullptr };
};
