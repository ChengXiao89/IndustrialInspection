#include "camera_view.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QJsonArray>

////////////////////////////////////////////////////////////////////////////////////////////////
camera_tree_widget::camera_tree_widget(QWidget* parent) :
    QTreeWidget(parent)
{

}

camera_tree_widget::~camera_tree_widget()
{

}

void camera_tree_widget::resizeEvent(QResizeEvent* event)
{
    QTreeWidget::resizeEvent(event);
    if (topLevelItemCount() > 0)
    {
        int height = visualItemRect(topLevelItem(0)).height();
        setColumnWidth(0, viewport()->width() - height);             // 第二列设置为正方形
        setColumnWidth(1, height);                                   // 第二列设置为正方形
    }
}

void camera_tree_widget::initialize()
{
    setHeaderHidden(true);      //隐藏表头
    setColumnCount(2);        //2列，第一列文本,第二列按钮按钮
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 强制不出现横向滚动条

    //图标资源
    m_icon_opened = QIcon("./icons/camera-opened.png");
    m_icon_closed = QIcon("./icons/camera-closed.png");
    m_icon_disabled = QIcon("./icons/camera-disabled.png");
    m_icon_refresh = QIcon("./icons/camera-refresh.png");

    if (m_root_item == nullptr)      //添加根节点
    {
        m_root_item = new QTreeWidgetItem(this);
        m_root_item->setText(0, QString::fromStdString("相机列表"));
        m_root_item->setIcon(1, m_icon_refresh);
        addTopLevelItem(m_root_item);
    }
    //注册消息
    connect(this, &QTreeWidget::itemClicked, this, &camera_tree_widget::on_item_clicked);

}

void camera_tree_widget::update_item_icon(const QString& unique_id)
{
	m_current_unique_id = unique_id; //记录当前打开的相机唯一标识符
	for (int i = 0;i < m_root_item->childCount();i++)
	{
        QTreeWidgetItem* cam_item = m_root_item->child(i);
        if (cam_item == nullptr)
        {
            continue;
        }
		if (m_current_unique_id == cam_item->text(0))
		{
			cam_item->setIcon(1, m_icon_opened);
		}
		else
		{
            cam_item->setIcon(1, m_icon_closed);
		}
	}
}

void camera_tree_widget::update_camera_list(const QJsonObject& obj)
{
	std::vector<st_camera_info*> cam_infos = json_to_device_list(obj);
	update_device_list(cam_infos);           //更新数据 m_device_list
	update_tree();                              //更新树控件
    //更新信息窗口
    camera_info_widget* info_widget = dynamic_cast<camera_info_widget*>(m_widget);
    if (info_widget != nullptr)
    {
        if(!is_unique_id_in_tree(info_widget->m_unique_id))
        {
            info_widget->update_camera_info(nullptr);
        }
    }
}

std::vector<st_camera_info*> camera_tree_widget::json_to_device_list(const QJsonObject& root)
{
    std::vector<st_camera_info*> device_list;
    if (!root.contains("device_list") || !root["device_list"].isArray())
        return device_list;

    QJsonArray devices_array = root["device_list"].toArray();

    for (const auto& device_val : devices_array)
    {
        if (!device_val.isObject())
            continue;

        QJsonObject device_obj = device_val.toObject();
        st_camera_info* cam_info = new st_camera_info();

        if (device_obj.contains("unique_id") && device_obj["unique_id"].isString())
            cam_info->m_unique_id = device_obj["unique_id"].toString();

        if (device_obj.contains("cam_items") && device_obj["cam_items"].isArray())
        {
            QJsonArray cam_items_array = device_obj["cam_items"].toArray();

            for (const auto& cam_val : cam_items_array)
            {
                if (!cam_val.isObject())
                    continue;

                QJsonObject cam_obj = cam_val.toObject();

                st_cam_item cam_item;
                if (cam_obj.contains("key") && cam_obj["key"].isString())
                    cam_item.m_key = cam_obj["key"].toString();

                if (cam_obj.contains("value") && cam_obj["value"].isString())
                    cam_item.m_value = cam_obj["value"].toString();

                cam_info->m_items.push_back(cam_item);
            }
        }

        device_list.push_back(cam_info);
    }
    return device_list;
}

void camera_tree_widget::update_device_list(std::vector<st_camera_info*>& new_device_list)
{
	//移除 m_device_list 中不包含于 device_list 的部分
    for (QMap<QString, st_camera_info*>::iterator iter = m_device_list.begin(); iter != m_device_list.end();)
    {
        bool should_erase = false;
        if (iter.value() == nullptr)
        {
            should_erase = true;
        }
        else
        {
            st_camera_info* cam_info = iter.value();
            //通过唯一标识检查当前相机数据是否存在于枚举结果中，以判定其是否仍与设备相连
            bool is_exist(false);
            for (int j = 0; j < static_cast<int>(new_device_list.size()); j++)
            {
                st_camera_info* new_cam_info = new_device_list[j];
                if (new_cam_info == nullptr)
                {
                    continue;
                }
                if (new_cam_info->m_unique_id == cam_info->m_unique_id)
                {
                    is_exist = true;
                    break;
                }
            }
            if (!is_exist)
            {
                should_erase = true;
            }
            
        }
        if (should_erase)
        {
            delete iter.value();
            iter = m_device_list.erase(iter); // 返回下一个有效迭代器
        }
        else
        {
            ++iter;
        }
    }

    //将 new_device_list 中不包含于 m_device_list 的部分添加到 m_device_list
    for (st_camera_info* new_cam_info : new_device_list)
    {
        if (new_cam_info == nullptr)
        {
            continue;
        }
        if (!m_device_list.contains(new_cam_info->m_unique_id))
        {
            m_device_list.insert(new_cam_info->m_unique_id, new st_camera_info(*new_cam_info));
        }
    }
}

void camera_tree_widget::update_tree()
{
    //移除树控件中无效节点
    for (int i = 0; i < m_root_item->childCount(); i++)
    {
        QTreeWidgetItem* cam_item = m_root_item->child(i);
        if (cam_item == nullptr)
        {
            continue;
        }
        QString unique_id = cam_item->text(0);
        QMap<QString, st_camera_info*>::iterator iter = m_device_list.find(unique_id);
        if (iter == m_device_list.end() || iter.value() == nullptr)
        {
            m_root_item->removeChild(cam_item);
            continue;
        }
    }
    //将新设备添加到树控件
    for (QMap<QString, st_camera_info*>::iterator iter = m_device_list.begin(); iter != m_device_list.end(); ++iter)
    {
        if (!is_unique_id_in_tree(iter.key()))
        {
            add_camera_item(iter.value());
        }
    }
    m_root_item->setExpanded(true); //展开根节点
}

bool camera_tree_widget::is_unique_id_in_tree(const QString& unique_id) const
{
    for (int i = 0; i < m_root_item->childCount(); i++)
    {
        QTreeWidgetItem* cam_item = m_root_item->child(i);
        if (cam_item == nullptr)
        {
            continue;
        }
        QString id = cam_item->text(0);
        if (id == unique_id)
        {
            return true;
        }
    }
    return false;
}

void camera_tree_widget::add_camera_item(const st_camera_info* cam_info)
{
    QTreeWidgetItem* lpItem = new QTreeWidgetItem(m_root_item);
    lpItem->setText(0, cam_info->m_unique_id);
    lpItem->setToolTip(0, cam_info->m_unique_id);
    lpItem->setIcon(1, m_icon_closed);
}

void camera_tree_widget::on_item_clicked(QTreeWidgetItem* item, int column)
{
    //点击根节点刷新图标,刷新所有连接设备
    if (item == m_root_item)
    {
        if(column == 0)
        {
            camera_info_widget* info_widget = dynamic_cast<camera_info_widget*>(m_widget);
            if (info_widget != nullptr)
            {
                info_widget->update_camera_info(nullptr);
            }
        }
        else if (column == 1)
        {
			emit post_request_update_camera_list();     //发送刷新相机列表的请求
        }
    }
    //点击相机节点
    else if (item->parent() == m_root_item)
    {
        if (column == 0)
        {
            camera_info_widget* info_widget = dynamic_cast<camera_info_widget*>(m_widget);
            if (info_widget != nullptr)
            {
                QString unique_id = item->text(0);
                QMap<QString, st_camera_info*>::iterator it = m_device_list.find(unique_id);
                if (it == m_device_list.end())
                {
                    info_widget->update_camera_info(nullptr);
                }
                info_widget->update_camera_info(it.value());
            }
        }
        else if (column == 1)
        {
            //打开相机
            QString unique_id = item->text(0);
            if(unique_id == m_current_unique_id)
            {
	            //关闭相机
                emit post_request_close_camera();
            }
            else
            {
                emit post_request_open_camera(QVariant::fromValue(unique_id)); //发送打开相机的请求
            }
			
        }
    }

}


////////////////////////////////////////////////////////////////////////////////////////////////
camera_info_widget::camera_info_widget(QWidget* parent)
    : QTableWidget(parent)
{

}

camera_info_widget::~camera_info_widget()
{
    // 清理资源
    setRowCount(0);
}

void camera_info_widget::initialize()
{
    setColumnCount(2);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
}

void camera_info_widget::update_camera_info(const st_camera_info* cam_info)
{
    if(cam_info == nullptr)
    {
        m_unique_id = QString("");
        setRowCount(0);
        return;
    }
    else
    {
        if (m_unique_id == cam_info->m_unique_id)
        {
            return;                                 //如果相机信息未改变，则不更新
        }
        setRowCount(0);
        m_unique_id = cam_info->m_unique_id;        //更新唯一标识符
        for (const auto& item : cam_info->m_items)
        {
            int row = rowCount();
            insertRow(row);
            setItem(row, 0, new QTableWidgetItem(item.m_key));
            setItem(row, 1, new QTableWidgetItem(item.m_value));
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////
camera_view_dock_widget::camera_view_dock_widget(fiber_end_client* client, QWidget* parent)
	:QDockWidget(parent),m_client(client)
{
    
}

camera_view_dock_widget::~camera_view_dock_widget()
{
	
}

void camera_view_dock_widget::initialize()
{
    QWidget* inner = new QWidget(this);
    QSplitter* splitter = new QSplitter(Qt::Vertical, inner);
    m_camera_tree_widget = new camera_tree_widget(splitter);
    m_camera_tree_widget->initialize();
	//向 dockwidget 发消息，dockwidget 使用客户端向后端发送请求
    connect(m_camera_tree_widget, &camera_tree_widget::post_request_update_camera_list, this, &camera_view_dock_widget::on_request_update_camera_list);
	connect(m_camera_tree_widget, &camera_tree_widget::post_request_open_camera, this, &camera_view_dock_widget::on_request_open_camera);
    connect(m_camera_tree_widget, &camera_tree_widget::post_request_close_camera, this, &camera_view_dock_widget::on_request_close_camera);
	m_camera_info_widget = new camera_info_widget(splitter);
    m_camera_info_widget->initialize();
    m_camera_tree_widget->set_info_widget(m_camera_info_widget); //设置树控件的相机信息控件
    QVBoxLayout* layout = new QVBoxLayout(inner);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);
    setWidget(inner);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);

    //客户端向 dockwidget 发消息，dockwidget 更新子控件以及向主窗口发消息更新界面
    connect(m_client, &fiber_end_client::post_update_camera_status, this, &camera_view_dock_widget::on_update_camera_status);
    connect(m_client, &fiber_end_client::post_update_camera_list, this, &camera_view_dock_widget::on_update_camera_list);
    connect(m_client, &fiber_end_client::post_camera_opened_success, this, &camera_view_dock_widget::on_camera_opened_success);
    connect(m_client, &fiber_end_client::post_camera_closed_success, this, &camera_view_dock_widget::on_camera_closed_success);

}

void camera_view_dock_widget::on_update_camera_status(const QVariant& data)
{
    QJsonObject obj = data.toJsonObject();
	//1.更新相机列表
	m_camera_tree_widget->update_camera_list(obj);

    QJsonObject camera_obj = obj["param"].toObject();
    if(camera_obj.contains(QString("unique_id")))
    {
        //2.更新相机状态图标
        QString unique_id = camera_obj["unique_id"].toString();
        m_camera_tree_widget->update_item_icon(unique_id);          //更新所有节点图标
		//3.向主程序发送消息，显示控制面板并更新相机参数界面
        emit post_camera_opened_success(obj); //向主窗口发送信号，打开控制面板
    }
	
}

void camera_view_dock_widget::on_update_camera_list(const QVariant& data)
{
	QJsonObject obj = data.toJsonObject();
    m_camera_tree_widget->update_camera_list(obj);
}

void camera_view_dock_widget::on_camera_opened_success(const QVariant& data)
{
    //1.更新图标状态
    QJsonObject obj = data.toJsonObject();
    QString unique_id = obj["unique_id"].toString();
	m_camera_tree_widget->update_item_icon(unique_id);          //更新所有节点图标
	//2.向主程序发送消息，显示控制面板并更新相机参数界面
	emit post_camera_opened_success(obj); //向主窗口发送信号，打开控制面板
}

void camera_view_dock_widget::on_camera_closed_success()
{
    //1.更新图标状态
    m_camera_tree_widget->update_item_icon("");
    //2.向主程序发送消息，清空相机参数界面并关闭控制面板
    emit post_camera_closed_success(); //向主窗口发送信号,关闭控制面板
}

void camera_view_dock_widget::on_request_update_camera_list()
{
    QJsonObject obj;
    obj["command"] = "client_request_camera_list";
    obj["param"] = 0;
    obj["request_id"] = fiber_end_client::generateUniqueRequestId(); // 生成唯一请求 ID
	m_client->send_message(obj);
    
}

void camera_view_dock_widget::on_request_open_camera(const QVariant& data)
{
    QJsonObject obj;
    obj["command"] = "client_request_open_camera";
    obj["param"] = data.value<QString>();
    obj["request_id"] = fiber_end_client::generateUniqueRequestId(); // 生成唯一请求 ID
    m_client->send_message(obj);
}

void camera_view_dock_widget::on_request_close_camera()
{
    QJsonObject obj;
    obj["command"] = "client_request_close_camera";
    obj["param"] = "";
    obj["request_id"] = fiber_end_client::generateUniqueRequestId(); // 生成唯一请求 ID
    m_client->send_message(obj);
}