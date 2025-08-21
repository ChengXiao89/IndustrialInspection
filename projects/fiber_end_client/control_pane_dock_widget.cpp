#include "control_pane_dock_widget.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QJsonObject>

control_pane_dock_widget::control_pane_dock_widget(fiber_end_client* client, QWidget* parent)
	:QDockWidget(parent), m_client(client)
{
	
}

control_pane_dock_widget::~control_pane_dock_widget()
{
	
}

void control_pane_dock_widget::initialize()
{
    QWidget* inner = new QWidget(this);
    QSplitter* splitter = new QSplitter(Qt::Horizontal, inner);
    m_control_tree_widget = new control_tree_widget(splitter);
    m_control_tree_widget->initialize();
    m_control_stacked_widget = new control_stacked_widget(splitter);
    m_control_stacked_widget->initialize();
    //m_control_stacked_widget 包含两个子窗口，第一个子窗口是相机参数窗口，第二个子窗口是算法参数窗口
    camera_parameter_widget* widget = dynamic_cast<camera_parameter_widget*>(m_control_stacked_widget->widget(0));
    connect(widget, &camera_parameter_widget::post_change_parameter, this, &control_pane_dock_widget::on_request_change_parameter);
    connect(widget, &camera_parameter_widget::post_start_grab, this, &control_pane_dock_widget::on_request_start_grab);
    connect(widget, &camera_parameter_widget::post_trigger_once, this, &control_pane_dock_widget::on_request_trigger_once);

    m_control_tree_widget->set_stacked_widget(m_control_stacked_widget); //设置树控件的相机信息控件
    //设置左右控件的宽度显示比例
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 6);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    QList<int> sizes;
    sizes << 4 << 6;
    splitter->setSizes(sizes);

    QVBoxLayout* layout = new QVBoxLayout(inner);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);
    setWidget(inner);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);

	//注册服务器返回的消息
    connect(m_client, &fiber_end_client::post_camera_parameter_changed_success, this, &control_pane_dock_widget::on_camera_parameter_changed_success);
    connect(m_client, &fiber_end_client::post_camera_grab_set_success, this, &control_pane_dock_widget::on_camera_grab_set_success);
    connect(m_client, &fiber_end_client::post_camera_trigger_once_success, this, &control_pane_dock_widget::on_camera_trigger_once_success);

}

void control_pane_dock_widget::update_camera_parameter(const QJsonObject& obj)
{
    m_control_stacked_widget->update_camera_parameter(obj); //更新相机参数
}

void control_pane_dock_widget::reset_camera_parameter()
{
	m_control_stacked_widget->reset_camera_parameter();     //清空相机参数
}

void control_pane_dock_widget::on_request_change_parameter(const QJsonObject& obj)
{
    QJsonObject object;
    object["command"] = "client_request_change_camera_parameter";
    object["param"] = obj;
    object["request_id"] = fiber_end_client::generateUniqueRequestId(); // 生成唯一请求 ID
    m_client->send_message(object);
}

void control_pane_dock_widget::on_request_start_grab(bool start)
{
    QJsonObject object;
    object["command"] = "client_request_start_grab";
    object["param"] = start;
    object["request_id"] = fiber_end_client::generateUniqueRequestId(); // 生成唯一请求 ID
    m_client->send_message(object);
}

void control_pane_dock_widget::on_request_trigger_once()
{
    QJsonObject object;
    object["command"] = "client_request_trigger_once";
    object["param"] = 0;
    object["request_id"] = fiber_end_client::generateUniqueRequestId(); // 生成唯一请求 ID
    m_client->send_message(object);
}


void control_pane_dock_widget::on_camera_parameter_changed_success(const QVariant& obj)
{
	QJsonObject object = obj.toJsonObject();
    m_control_stacked_widget->update_camera_parameter(object); //更新相机参数
}

void control_pane_dock_widget::on_camera_grab_set_success(const QVariant& obj)
{
    QJsonObject object = obj.toJsonObject();
	m_control_stacked_widget->update_camera_grab_status(object); //更新采集状态
}

void control_pane_dock_widget::on_camera_trigger_once_success(const QVariant& obj)
{
    QJsonObject object = obj.toJsonObject();
	//1. 解析服务端元数据，并从共享内存中读取影像
    QJsonObject meta_object = object["param"].toObject();
    st_image_meta meta = image_shared_memory::json_to_meta(meta_object);
    QImage img = m_image_shared_memory.read_image(meta);
    if (0 && !img.isNull()) 
    {
		img.save("D:/Temp/client.png"); // 测试保存图片
    }

    //2.向主程序发送消息，显示中央控件并显示影像
	emit post_camera_trigger_once_success(img); //向主窗口发送信号，打开中央控件并显示影像
    
}
