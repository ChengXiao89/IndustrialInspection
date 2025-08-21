#pragma once
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QVariant>

#include <vector>
using namespace std;

#include "../device_enum/device_enum_factory.h"
#include "../device_camera/camera_factory.h"


//设备节点数据，包含一个设备信息对象指针和相机对象指针
struct st_device_node
{
	st_device_info* m_device_info{ nullptr };	//基类对象指针，目前是MVS
	interface_camera* m_camera{ nullptr };		//基类对象指针，目前是MVS

	bool create_camera()
	{
		if(m_camera == nullptr)
		{
			m_camera = camera_factory::create_camera(m_device_info);
		}
		return  m_camera != nullptr;
	}

	~st_device_node()
	{
		if(m_device_info)
		{
			delete m_device_info;
			m_device_info = nullptr;
		}
		if (m_camera)
		{
			m_camera->close();		//相机可能正在处理耗时任务，这里可能需要进行一些较复杂的资源释放操作，详细情况需要参考SDK
			delete m_camera;
			m_camera = nullptr;
		}
	}
};

/***************************
 * 相机树控件. 维护所有相机设备
 * (1) 启动时自动检测所有连接的相机设备，在树控件中显示
 * (2) 定时线程，每隔一秒自动检测相机设备的连接状态，更新列表
 * (3) 设备的连接状态由外部控制，不支持主动移除和连接
 * (4) 树控件支持打开相机以及设置相机 ip 地址，一次只能打开一个相机，每次打开相机时必须先关闭之前打开的相机
 *****************************/
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

	// 初始化设备信息和树控件
	void initialize_camera_list();
	// 向树控件中添加相机节点
	bool add_camera_item(st_device_node* device_node);

	// 打开/关闭相机之后,更新所有节点图标,刷新树控件时也需要更新所有节点图标
	void update_item_icon();

	void update_camera_list();
	//刷新树控件时调用，在枚举设备之后，更新 m_device_node_list
	void update_node_list(vector<st_device_info*>& new_device_list);
	//刷新树控件时调用，更新 m_device_node_list 之后，更新树控件
	void update_tree();
	//查找树控件中是否存在对应节点
	bool is_unique_id_in_tree(const QString& unique_id) const;

	//访问成员函数的接口
	st_device_node* current_device() const{ return m_current_device; }		//返回当前打开的相机

private slots:
	void on_item_clicked(QTreeWidgetItem* item, int column);
	void on_item_double_clicked(QTreeWidgetItem* item, int column);
	void on_custom_context_menu_requested(const QPoint& pos);			//右键菜单请求
	void on_set_camera_ip();

signals:
	void post_camera_opened(const QVariant& data);				//打开相机之后向主程序发送消息

private:
	TYPE_SDK m_sdk_type{ SDK_DVP2 };
	QMenu m_menu;											 //右键菜单
	//QAction* m_action_open_camera{ nullptr };				 //打开相机
	QAction* m_action_set_ip_config{ nullptr };				 //设置 ip
	QTreeWidgetItem* m_selected_item{ nullptr };			 //弹出右键菜单时选中的项
	QTreeWidgetItem* m_root_item{ nullptr };                 //树控件根节点，管理所有相机节点
	QTreeWidgetItem* m_GigE_root_item{ nullptr };            //管理所有 GigE 类型相机节点
	QTreeWidgetItem* m_usb_root_item{ nullptr };             //管理所有 USB 类型相机节点
	QTreeWidgetItem* m_cam_link_root_item{ nullptr };        //管理所有 Camera Link 类型相机节点
	QTreeWidgetItem* m_cxp_root_item{ nullptr };             //管理所有 CXP 类型相机节点
	QTreeWidgetItem* m_xof_root_item{ nullptr };             //管理所有 XOF 类型相机节点

	QMap<QString, st_device_node*> m_device_node_list;		 //存储所有相机对象以及相机设备信息,相机节点存储的是唯一标识，
															 //通过唯一标识找到对应相机
	st_device_node* m_current_device{ nullptr };			 //当前打开的相机, 
	QWidget* m_widget{ nullptr };							 //关联控件，相机信息显示控件

	QIcon m_icon_opened;		//表示设备处于打开状态
	QIcon m_icon_closed;		//表示设备处于关闭状态
	QIcon m_icon_disabled;		//表示设备处于不可用状态(被其他程序打开)
	QIcon m_icon_refresh;		//刷新连接设备及其状态
};