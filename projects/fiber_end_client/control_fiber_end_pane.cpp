#include "control_fiber_end_pane.h"

#include <QLabel>
#include <QBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QJsonObject>
#include <QJsonArray>

control_fiber_end_pane::control_fiber_end_pane(QWidget* parent):
	QWidget(parent)
{
	
}

control_fiber_end_pane::~control_fiber_end_pane()
{
	
}

void control_fiber_end_pane::initialize()
{
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(10, 10, 10, 10);

    //运动控制
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->setVerticalSpacing(5);
        formLayout->setHorizontalSpacing(10);
        //光源亮度
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_light_brightness = new QLabel("光源亮度:");
            m_edit_light_brightness = new QLineEdit(this);
            h_layout->addWidget(label_light_brightness);
            h_layout->addWidget(m_edit_light_brightness);
            formLayout->addRow(h_layout);
        }
        //运动速度
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_move_speed = new QLabel("运动速度:");
            m_edit_move_speed = new QLineEdit(this);
            h_layout->addWidget(label_move_speed);
            h_layout->addWidget(m_edit_move_speed);
            formLayout->addRow(h_layout);
        }

        // 设备位置
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_position_x = new QLabel("相机位置(X):");
            m_edit_position_x = new QLineEdit(this);
            QLabel* label_position_y = new QLabel("相机位置(Y):");
            m_edit_position_y = new QLineEdit();
            h_layout->addWidget(label_position_x);
            h_layout->addWidget(m_edit_position_x);
            h_layout->addWidget(label_position_y);
            h_layout->addWidget(m_edit_position_y);
            formLayout->addRow(h_layout);
        }
    	//移动到指定位置以及设置为零点
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            m_push_button_move_to_position = new QPushButton("移动到相机位置");
            connect(m_push_button_move_to_position, &QPushButton::clicked, this, &control_fiber_end_pane::on_move_to_position);
            m_push_button_set_position_zero = new QPushButton("相机位置设为零点");
            connect(m_push_button_set_position_zero, &QPushButton::clicked, this, &control_fiber_end_pane::on_set_current_position_zero);
            h_layout->addWidget(m_push_button_move_to_position);
            h_layout->addWidget(m_push_button_set_position_zero);
            formLayout->addRow(h_layout);
        }
        //横向移动距离与纵向移动距离
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_step_x = new QLabel("移动步长(X):");
            m_edit_move_step_x = new QLineEdit(this);
            QLabel* label_step_y = new QLabel("移动步长(Y):");
            m_edit_move_step_y = new QLineEdit();
            h_layout->addWidget(label_step_x);
            h_layout->addWidget(m_edit_move_step_x);
            h_layout->addWidget(label_step_y);
            h_layout->addWidget(m_edit_move_step_y);
            formLayout->addRow(h_layout);
        }
        //上下左右按钮
        {
            //QWidget* widget = new QWidget();
            QVBoxLayout* layout = new QVBoxLayout();
            QSize button_size(64, 64);       // 按钮大小固定

            // 第一行: 向上的按钮,居中
            QHBoxLayout* h_layout_1 = new QHBoxLayout();
            h_layout_1->addStretch();
            QIcon icon_up("./icons/move_up.png");
            m_push_button_move_forward_y = create_push_button(button_size, icon_up);
            connect(m_push_button_move_forward_y, &QPushButton::clicked, this, &control_fiber_end_pane::on_move_forward_y);
            h_layout_1->addWidget(m_push_button_move_forward_y);
            h_layout_1->addStretch();
            layout->addLayout(h_layout_1);

            // 第二行: 向左与向右按钮居中，中间留 button_size 尺寸的空白
            QHBoxLayout* h_layout_2 = new QHBoxLayout();
            h_layout_2->addStretch();
            QIcon icon_left("./icons/move_left.png");
            m_push_button_move_back_x = create_push_button(button_size, icon_left);
            connect(m_push_button_move_back_x, &QPushButton::clicked, this, &control_fiber_end_pane::on_move_back_x);
            h_layout_2->addWidget(m_push_button_move_back_x);

            h_layout_2->addSpacing(button_size.width());  // 中间留白

            QIcon icon_right("./icons/move_right.png");
            m_push_button_move_forward_x = create_push_button(button_size, icon_right);
            connect(m_push_button_move_forward_x, &QPushButton::clicked, this, &control_fiber_end_pane::on_move_forward_x);
            h_layout_2->addWidget(m_push_button_move_forward_x);
            h_layout_2->addStretch();
            layout->addLayout(h_layout_2);

            // 第三行: 向下的按钮,居中
            QHBoxLayout* h_layout_3 = new QHBoxLayout();
            h_layout_3->addStretch();
            QIcon icon_down("./icons/move_down.png");
            m_push_button_move_back_y = create_push_button(button_size, icon_down);
            connect(m_push_button_move_back_y, &QPushButton::clicked, this, &control_fiber_end_pane::on_move_back_y);
            h_layout_3->addWidget(m_push_button_move_back_y);
            h_layout_3->addStretch();
            layout->addLayout(h_layout_3);

            formLayout->addRow(layout);
        }
        //自动对焦按钮
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            m_push_button_auto_focus = new QPushButton("自动对焦");
            connect(m_push_button_auto_focus, &QPushButton::clicked, this, &control_fiber_end_pane::on_auto_focus);
            h_layout->addWidget(m_push_button_auto_focus);
            formLayout->addRow(h_layout);
        }
        QGroupBox* motion_control_group = new QGroupBox("运动控制");
        motion_control_group->setLayout(formLayout);
        main_layout->addWidget(motion_control_group);
    }

    //参数设置
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->setVerticalSpacing(5);
        formLayout->setHorizontalSpacing(10);

        // Y 轴位置列表
        {
            m_position_list = new QListWidget();
            m_position_list->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
            
            // 插入第一行作为标题
            QListWidgetItem* item = new QListWidgetItem("Y 轴位置列表");
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // 去掉可编辑属性
            m_position_list->addItem(item);

            formLayout->addRow(m_position_list);

            // 按钮
            QHBoxLayout* h_layout = new QHBoxLayout();
            m_push_button_add_position = new QPushButton("添加");
            connect(m_push_button_add_position, &QPushButton::clicked, this, &control_fiber_end_pane::on_add_y_position);
            m_push_button_remove_position = new QPushButton("移除");
            connect(m_push_button_remove_position, &QPushButton::clicked, this, &control_fiber_end_pane::on_remove_y_position);
            h_layout->addWidget(m_push_button_add_position);
            h_layout->addWidget(m_push_button_remove_position);
            formLayout->addRow(h_layout);
        }
        // 影像端面个数和自动检测开关
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_fiber_count = new QLabel("影像端面个数 :");
            m_edit_fiber_count = new QLineEdit();
            m_check_auto_detect = new QCheckBox("自动检测");
            connect(m_check_auto_detect, &QCheckBox::checkStateChanged, this, &control_fiber_end_pane::on_auto_detect_set_changed);
            h_layout->addWidget(label_fiber_count);
            h_layout->addWidget(m_edit_fiber_count);
            h_layout->addWidget(m_check_auto_detect);
            formLayout->addRow(h_layout);
        }
        // 影像保存路径
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_save_path = new QLabel("保存路径 :");
            m_edit_image_save_path = new QLineEdit();
            m_push_button_set_image_save_path = new QPushButton("浏览");
            connect(m_push_button_set_image_save_path, &QPushButton::clicked, this, &control_fiber_end_pane::on_set_image_save_path);
            h_layout->addWidget(label_save_path);
            h_layout->addWidget(m_edit_image_save_path);
            h_layout->addWidget(m_push_button_set_image_save_path);
            formLayout->addRow(h_layout);
        }
        QGroupBox* parameter_group = new QGroupBox("参数设置");
        parameter_group->setLayout(formLayout);
        main_layout->addWidget(parameter_group);
    }
    //开始停止
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->setVerticalSpacing(5);
        formLayout->setHorizontalSpacing(10);
        //开始停止
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            m_push_button_start = new QPushButton("开始运行");
            connect(m_push_button_start, &QPushButton::clicked, this, &control_fiber_end_pane::on_start);
            m_push_button_stop = new QPushButton("停止运行");
            connect(m_push_button_stop, &QPushButton::clicked, this, &control_fiber_end_pane::on_stop);
            h_layout->addWidget(m_push_button_start);
            h_layout->addWidget(m_push_button_stop);
            formLayout->addRow(h_layout);
        }
        QGroupBox* run_group = new QGroupBox("运行控制");
        run_group->setLayout(formLayout);
        main_layout->addWidget(run_group);
    }
    main_layout->addStretch(); // 添加弹性空间
}

void control_fiber_end_pane::update_parameter(const QJsonObject& obj)
{
    QJsonObject root = obj["parameter"].toObject();
    m_light_brightness = root["light_brightness"].toInt();
    m_edit_light_brightness->setText(QString("%1").arg(m_light_brightness));
    m_move_peed = root["move_speed"].toInt();
    m_edit_move_speed->setText(QString("%1").arg(m_move_peed));
    m_move_step_x = root["move_step_x"].toInt();
    m_edit_move_step_x->setText(QString("%1").arg(m_move_step_x));
    m_move_step_y = root["move_step_y"].toInt();
    m_edit_move_step_y->setText(QString("%1").arg(m_move_step_y));

    QJsonArray posArray = root["position_list"].toArray();
    for (int i = 0; i < posArray.size(); i++)
    {
        int value = posArray[i].toInt();
        m_position_list->addItem(QString("%1").arg(value));
    }

    m_fiber_end_count = root["fiber_end_count"].toInt();
    m_edit_fiber_count->setText(QString("%1").arg(m_fiber_end_count));

    m_auto_detect = root["auto_detect"].toInt();
    if(m_auto_detect == 0)
    {
        m_check_auto_detect->setCheckState(Qt::Unchecked);
    }
    else
    {
        m_check_auto_detect->setCheckState(Qt::Checked);
    }
    m_edit_image_save_path->setText(root["save_path"].toString());
    
}

QPushButton* control_fiber_end_pane::create_push_button(const QSize& button_size, const QIcon& icon)
{
    QPushButton* button = new QPushButton();
    button->setFixedSize(button_size);
    button->setIcon(icon);
    button->setIconSize(button_size);
    return button;
}

void control_fiber_end_pane::on_move_to_position()
{
	
}

void control_fiber_end_pane::on_set_current_position_zero()
{
	
}

void control_fiber_end_pane::on_move_forward_y()
{
	
}

void control_fiber_end_pane::on_move_back_x()
{
	
}

void control_fiber_end_pane::on_move_forward_x()
{
	
}

void control_fiber_end_pane::on_move_back_y()
{
	
}

void control_fiber_end_pane::on_auto_focus()
{
	
}

void control_fiber_end_pane::on_add_y_position()
{
	
}

void control_fiber_end_pane::on_remove_y_position()
{
	
}

void control_fiber_end_pane::on_auto_detect_set_changed(int check_state)
{
	
}

void control_fiber_end_pane::on_set_image_save_path()
{
	
}

void control_fiber_end_pane::on_start()
{
	
}

void control_fiber_end_pane::on_stop()
{
	
}