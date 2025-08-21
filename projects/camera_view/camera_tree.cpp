#include <QMessageBox>
#include <QHeaderView>


#include "camera_tree.h"
#include "camera_info.h"
#include "ip_config_dialog.h"

camera_tree_widget::camera_tree_widget(QWidget* parent):
	QTreeWidget(parent)
{
	
}

camera_tree_widget::~camera_tree_widget()
{
    if(m_current_device != nullptr)
    {
        m_current_device->m_camera->close();
    }
    for (QMap<QString, st_device_node*>::iterator iter = m_device_node_list.begin(); iter != m_device_node_list.end(); ++iter)
	{
		if (iter.value() != nullptr)
		{
            delete iter.value();
		}
	}
    m_device_node_list.clear();

    device_enum_factory::release_device_enums();
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
    m_icon_opened = QIcon("./../icons/camera-opened.png");
    m_icon_closed = QIcon("./../icons/camera-closed.png");
    m_icon_disabled = QIcon("./../icons/camera-disabled.png");
    m_icon_refresh = QIcon("./../icons/camera-refresh.png");

    if (m_root_item == nullptr)      //添加根节点
    {
        m_root_item = new QTreeWidgetItem(this);
        m_root_item->setText(0, QString::fromStdString("相机列表"));
        m_root_item->setIcon(1, m_icon_refresh);
        addTopLevelItem(m_root_item);
    }
    //注册消息
    connect(this, &QTreeWidget::itemClicked, this, &camera_tree_widget::on_item_clicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &camera_tree_widget::on_item_double_clicked);
    //右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &camera_tree_widget::on_custom_context_menu_requested);

    //m_action_open_camera = m_menu.addAction(QString::fromStdString("打开"));
    //connect(m_action_open_camera, &QAction::triggered, this, &camera_tree_widget::on_open_camera);
    m_action_set_ip_config = m_menu.addAction(QString::fromStdString("设置IP地址"));
    connect(m_action_set_ip_config, &QAction::triggered, this, &camera_tree_widget::on_set_camera_ip);

    //初始化获取所有连接的相机，显示在树控件中
    initialize_camera_list();
}

void camera_tree_widget::initialize_camera_list()
{
    interface_device_enum* device_enum = device_enum_factory::create_device_enum(m_sdk_type);
    vector<st_device_info*> device_info_list = device_enum->enumerate_devices();
    for (size_t i = 0; i < device_info_list.size(); i++)
    {
        st_device_info* device_info = device_info_list[i];
        if (device_info == nullptr)
        {
            continue;
        }
        st_device_node* device_node = new st_device_node;
        device_node->m_device_info = device_info;
        if(add_camera_item(device_node))
        {
            m_device_node_list.insert(device_node->m_device_info->m_unique_id, device_node);
        }
        else
        {
            delete device_node;
        }
    }
    m_root_item->setExpanded(true); //展开根节点
    for (int i = 0; i < m_root_item->childCount(); i++)
    {
        m_root_item->child(i)->setExpanded(true); //展开所有子节点
    }
}

bool camera_tree_widget::add_camera_item(st_device_node* device_node)
{
    if(device_node == nullptr || device_node->m_device_info == nullptr)
    {
	    return false;
    }
    QTreeWidgetItem* parent_item(nullptr);
    int device_type = device_node->m_device_info->get_device_type();
    if (device_type == TYPE_DEVICE_GIGE)
    {
        if (m_GigE_root_item == nullptr)
        {
            m_GigE_root_item = new QTreeWidgetItem(m_root_item);
            m_GigE_root_item->setFirstColumnSpanned(true);
            m_GigE_root_item->setText(0, "GigE");
        }
        parent_item = m_GigE_root_item;
    }
    else if (device_type == TYPE_DEVICE_USB)
    {
        if (m_usb_root_item == nullptr)
        {
            m_usb_root_item = new QTreeWidgetItem(m_root_item);
            m_usb_root_item->setFirstColumnSpanned(true);
            m_usb_root_item->setText(0, "USB");
        }
        parent_item = m_usb_root_item;
    }
    else if (device_type == TYPE_DEVICE_GENTL_CAMERALINK)
    {
        if (m_cam_link_root_item == nullptr)
        {
            m_cam_link_root_item = new QTreeWidgetItem(m_root_item);
            m_cam_link_root_item->setFirstColumnSpanned(true);
            m_cam_link_root_item->setText(0, "CAM_LINK");
        }
        parent_item = m_cam_link_root_item;
    }
    else if (device_type == TYPE_DEVICE_GENTL_CXP)
    {
        if (m_cxp_root_item == nullptr)
        {
            m_cxp_root_item = new QTreeWidgetItem(m_root_item);
            m_cxp_root_item->setFirstColumnSpanned(true);
            m_cxp_root_item->setText(0, "CXP");
        }
        parent_item = m_cxp_root_item;
    }
    else if (device_type == TYPE_DEVICE_GENTL_XOF)
    {
        if (m_xof_root_item == nullptr)
        {
            m_xof_root_item = new QTreeWidgetItem(m_root_item);
            m_xof_root_item->setFirstColumnSpanned(true);
            m_xof_root_item->setText(0, "XOF");
        }
        parent_item = m_xof_root_item;
    }
    if(parent_item == nullptr)
    {
	    return false;
    }
    QTreeWidgetItem* lpItem = new QTreeWidgetItem(parent_item);
    lpItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(device_node->m_device_info->m_unique_id));
    QString text = device_node->m_device_info->get_item(QString("序列号")) + "(" + 
					device_node->m_device_info->get_item(QString("设备型号")) + ")";
    lpItem->setText(0, text);
    lpItem->setToolTip(0, text);
    lpItem->setIcon(1, m_icon_closed);
    return true;
}

void camera_tree_widget::on_custom_context_menu_requested(const QPoint& pos)
{
    QTreeWidgetItem* item = this->itemAt(pos);  // 获取鼠标下的项
    if(item == nullptr || item == m_root_item || item->parent() == m_root_item)
    {
        m_selected_item = nullptr;
        return; //如果是根节点或没有选中项，则不显示菜单
	}
	m_selected_item = item; //记录选中的项
    if(m_selected_item->parent() == m_GigE_root_item)
    {
        m_action_set_ip_config->setVisible(true);
    }
    else
    {
        m_action_set_ip_config->setVisible(false);
    }
    m_menu.popup(QCursor::pos());
    
}

void camera_tree_widget::on_set_camera_ip()
{
	//获取 m_selected_item 对应的数据，修改相机 IP 地址。 MVS软件中如果相机被打开则不能修改 IP
    QString unique_id = m_selected_item->data(0, Qt::UserRole + 1).value<QString>();
    QMap<QString, st_device_node*>::iterator iter = m_device_node_list.find(unique_id);
    if(iter == m_device_node_list.end() || iter.value() == nullptr)
    {
        QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("无效数据!"), QMessageBox::Ok);
        return;
    }
    if(!iter.value()->create_camera())
    {
        QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("创建相机失败!"), QMessageBox::Ok);
        return;
    }
	QString str_ip_address = iter.value()->m_device_info->get_item(QString::fromStdString("IP地址"));
    QString str_subnet_mask = iter.value()->m_device_info->get_item(QString::fromStdString("子网掩码"));
    QString str_default_gate = iter.value()->m_device_info->get_item(QString::fromStdString("默认网关"));
	ip_config_dialog dlg(str_ip_address, str_subnet_mask, str_default_gate);
    dlg.exec();
    //静态 ip
    if(dlg.config_type == 0)
    {
        if(iter.value()->m_camera->set_ip_address(dlg.m_ip_address, dlg.m_subnet_mask, dlg.m_default_gateway) != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), 
                QString::fromStdString("ip 地址设置失败!"), QMessageBox::Ok);
        }
        else
        {
            //修改成功之后需要更新当前数据(lp_device_info)
            iter.value()->m_device_info->set_item("IP地址", dlg.m_str_ip_address);
            iter.value()->m_device_info->set_item("子网掩码", dlg.m_str_subnet_mask);
            iter.value()->m_device_info->set_item("默认网关", dlg.m_str_default_gateway);
            camera_info_widget* info_widget = dynamic_cast<camera_info_widget*>(m_widget);
            if (info_widget != nullptr)
            {
                info_widget->update_device_info(iter.value()->m_device_info);
            }
            QMessageBox::information(this, QString::fromStdString("提示"),
                QString::fromStdString("ip 地址设置成功!"), QMessageBox::Ok);
        }
    }
    //DHCP
	else if(dlg.config_type == 1 || dlg.config_type == 2)
    {
        int nRet(0);
        if(dlg.config_type == 1)
        {
            nRet = iter.value()->m_camera->set_ip_config(IP_CONFIG_DHCP);
        }
        else
        {
            nRet = iter.value()->m_camera->set_ip_config(IP_CONFIG_LLA);
        }
        if (nRet != STATUS_SUCCESS)
        {
            QString text = QString("配置失败: %1").arg(nRet);
            QMessageBox::information(this, QString::fromStdString("错误"), text, QMessageBox::Ok);
        }
        else
        {
	        //使用 DHCP/LLA 模式配置IP之后不会立即生效，需要重启设备
            QMessageBox::information(this, QString::fromStdString("提示"),
                QString::fromStdString("配置成功，为了确保修改生效，建议您重启设备!"), QMessageBox::Ok);
        }
    }
}

void camera_tree_widget::on_item_double_clicked(QTreeWidgetItem* item, int column)
{
    //QMessageBox::information(nullptr, QString::fromStdString("提示"), item->text(0), QMessageBox::Ok);
}

void camera_tree_widget::on_item_clicked(QTreeWidgetItem* item, int column)
{
    //点击根节点刷新图标,刷新所有连接设备
    if(item == m_root_item)
    {
        if(column == 1)
        {
            update_camera_list();
        }
    }
    //点击相机节点
    else if(item->parent()->parent() == m_root_item)
    {
        QString unique_id = item->data(0, Qt::UserRole + 1).value<QString>();
        QMap<QString, st_device_node*>::iterator iter = m_device_node_list.find(unique_id);
        if (iter == m_device_node_list.end() || iter.value() == nullptr)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("无效数据!"), QMessageBox::Ok);
            return;
        }
        if (column == 0)
        {
            camera_info_widget* info_widget = dynamic_cast<camera_info_widget*>(m_widget);
            if (item == nullptr || info_widget == nullptr)
            {
                return;
            }
            if (item == m_root_item || item->parent() == m_root_item)
            {
                info_widget->set_device_info(nullptr);
            }
            info_widget->set_device_info(iter.value()->m_device_info); //设置设备信息
        }
        else if (column == 1)
        {
            /********************************
             * 打开或关闭相机. 分为如下几种情况:
             * (1) 所有的相机都没有打开，此时打开指定相机
             * (2) 有其他相机被打开, 此时提示是否切换相机
             * (3) 关闭当前相机
             * 操作完成之后需要更新所有节点的图标状态（当前节点为opened,其他都是closed）
             ********************************/
            if (!iter.value()->create_camera())
            {
                QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("创建相机对象失败!"), QMessageBox::Ok);
                return;

            }
            //1. 所有相机均未打开,打开相机
            if (m_current_device == nullptr)
            {
                if (iter.value()->m_camera->open() != STATUS_SUCCESS)
                {
                    QMessageBox::information(this, QString::fromStdString("提示"),
                        QString::fromStdString("打开设备失败!"), QMessageBox::Ok);
                    return;
                }
                m_current_device = iter.value();      //打开成功，需要更新其他插件
            }
            //2. 已有相机被打开，询问是否切换
            else if (m_current_device != iter.value())
            {
                if (QMessageBox::information(this,
                    QString::fromStdString("询问"),
                    QString::fromStdString("只能打开一个相机，是否切换?"),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                {
                    //有可能正在使用当前打开的相机处理数据，需要清理资源
                    if (m_current_device->m_camera->close() != STATUS_SUCCESS)
                    {
                        QMessageBox::information(this,
                            QString::fromStdString("提示"),
                            QString::fromStdString("关闭当前设备失败!"),
                            QMessageBox::Ok);
                        return;
                    }
                    m_current_device = nullptr;
                    if (iter.value()->m_camera->open() != STATUS_SUCCESS)
                    {
                        QMessageBox::information(this,
                            QString::fromStdString("提示"),
                            QString::fromStdString("打开设备失败!"),
                            QMessageBox::Ok);
                        return;
                    }
                    m_current_device = iter.value();
                }
            }
            //3. 关闭当前打开的相机
            else if (m_current_device == iter.value())
            {
                if (m_current_device->m_camera->close() != STATUS_SUCCESS)
                {
                    QMessageBox::information(this,
                        QString::fromStdString("提示"),
                        QString::fromStdString("关闭当前设备失败!"),
                        QMessageBox::Ok);
                    return;
                }
                m_current_device = nullptr;
            }
            update_item_icon();

            /*********************
             * 打开或者关闭相机之后需要初始化或者更新另一个插件
             * 指针为空表示关闭，有效则表示打开
             *********************/
            if (m_current_device == nullptr)
            {
                emit post_camera_opened(QVariant::fromValue(static_cast<void*>(nullptr)));
            }
            else
            {
                emit post_camera_opened(QVariant::fromValue(static_cast<void*>(m_current_device->m_camera)));
            }

        }
    }

}

void camera_tree_widget::update_item_icon()
{
	for (int i = 0;i < m_root_item->childCount();i++)
	{
        QTreeWidgetItem* sub_root_item = m_root_item->child(i);
        if(sub_root_item == nullptr || sub_root_item->childCount() == 0)
        {
	        continue;
        }
        for (int j = 0;j < sub_root_item->childCount();j++)
        {
            QTreeWidgetItem* cam_item = sub_root_item->child(j);
            if (cam_item == nullptr)
            {
                continue;
            }
            QString unique_id = cam_item->data(0, Qt::UserRole + 1).value<QString>();
            QMap<QString, st_device_node*>::iterator iter = m_device_node_list.find(unique_id);
            if (iter == m_device_node_list.end() || iter.value() == nullptr)
            {
                continue;       //异常情况
            }
        	if(iter.value() != nullptr && iter.value() == m_current_device)
            {
                cam_item->setIcon(1, m_icon_opened);
            }
            else
            {
                cam_item->setIcon(1, m_icon_closed);
            }
        }
	}
}

void camera_tree_widget::update_camera_list()
{
    //枚举所有的设备
    interface_device_enum* device_enum = device_enum_factory::create_device_enum(m_sdk_type);
    vector<st_device_info*> device_info_list = device_enum->enumerate_devices();
    /**************************1.更新节点列表****************************/
    update_node_list(device_info_list);
    /*****************************2.更新树控件***************************/
    update_tree();
}

void camera_tree_widget::update_node_list(vector<st_device_info*>& new_device_list)
{
    //移除 m_device_node_list 中不包含于 new_device_list 的部分
    for (QMap<QString, st_device_node*>::iterator iter = m_device_node_list.begin(); iter != m_device_node_list.end();)
    {
        bool should_erase = false;
        if (iter.value() == nullptr)
        {
            should_erase = true;
        }
        else
        {
            st_device_info* device_info = iter.value()->m_device_info;
            if (device_info == nullptr)
            {
                should_erase = true;
            }
            else
            {
                //通过唯一标识检查当前相机数据是否存在于枚举结果中，以判定其是否仍与设备相连
                bool is_exist(false);
                for (int j = 0; j < new_device_list.size(); j++)
                {
                    st_device_info* new_device_info = new_device_list[j];
                    if (new_device_info == nullptr)
                    {
                        continue;
                    }
                    if (new_device_info->m_unique_id == device_info->m_unique_id)
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
        }
        if (should_erase) 
        {
            delete iter.value();
            iter = m_device_node_list.erase(iter); // 返回下一个有效迭代器
        }
        else 
        {
            ++iter;
        }
    }
    //将 new_device_list 中不包含于 m_device_node_list 的部分添加到 m_device_node_list
    for (int i = new_device_list.size() - 1; i >= 0; i--)
    {
        st_device_info* new_device_info = new_device_list[i];
        if (new_device_info == nullptr)
        {
            continue;
        }
        if(m_device_node_list.find(new_device_info->m_unique_id) == m_device_node_list.end())
        {
            st_device_node* new_device_node = new st_device_node;
            new_device_node->m_device_info = new_device_list[i];
            m_device_node_list.insert(new_device_info->m_unique_id, new_device_node);
            new_device_list.erase(new_device_list.begin() + i);
        }
    }

    //释放 new_device_list 资源
    for (int i = 0;i < new_device_list.size();i++)
    {
	    if(new_device_list[i] != nullptr)
	    {
            delete new_device_list[i];
            new_device_list[i] = nullptr;
	    }
    }
    new_device_list.clear();
}

void camera_tree_widget::update_tree()
{
	//先从树控件中移除无效节点
    for (int i = 0; i < m_root_item->childCount(); i++)
    {
        QTreeWidgetItem* sub_root_item = m_root_item->child(i);
        if (sub_root_item == nullptr || sub_root_item->childCount() == 0)
        {
            continue;
        }
        for (int j = 0; j < sub_root_item->childCount(); j++)
        {
            QTreeWidgetItem* cam_item = sub_root_item->child(j);
            if (cam_item == nullptr)
            {
                continue;
            }
            QString unique_id = cam_item->data(0, Qt::UserRole + 1).value<QString>();
            QMap<QString, st_device_node*>::iterator iter = m_device_node_list.find(unique_id);
            if (iter == m_device_node_list.end() || iter.value() == nullptr)
            {
                sub_root_item->removeChild(cam_item);
            }
        }
    }
    //将新设备添加到树控件
    for (QMap<QString, st_device_node*>::iterator iter = m_device_node_list.begin(); iter != m_device_node_list.end();++iter)
    {
	    if(!is_unique_id_in_tree(iter.key()))
	    {
            add_camera_item(iter.value());
	    }
    }
    m_root_item->setExpanded(true); //展开根节点
    for (int i = 0; i < m_root_item->childCount(); i++)
    {
        m_root_item->child(i)->setExpanded(true); //展开所有子节点
    }
}

bool camera_tree_widget::is_unique_id_in_tree(const QString& unique_id) const
{
    for (int i = 0; i < m_root_item->childCount(); i++)
    {
        QTreeWidgetItem* sub_root_item = m_root_item->child(i);
        if (sub_root_item == nullptr || sub_root_item->childCount() == 0)
        {
            continue;
        }
        for (int j = 0; j < sub_root_item->childCount(); j++)
        {
            QTreeWidgetItem* cam_item = sub_root_item->child(j);
            if (cam_item == nullptr)
            {
                continue;
            }
            QString id = cam_item->data(0, Qt::UserRole + 1).value<QString>();
            if(id == unique_id)
            {
	            return true;
            }
        }
    }
    return false;
}