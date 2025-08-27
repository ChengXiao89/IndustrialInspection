#include "control_fiber_end_pane.h"

#include <QLabel>
#include <QBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileDialog>

#include "../common/common.h"


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
            connect(m_edit_light_brightness, &QLineEdit::editingFinished, this, &control_fiber_end_pane::on_light_brightness_changed);
            h_layout->addWidget(label_light_brightness);
            h_layout->addWidget(m_edit_light_brightness);
            formLayout->addRow(h_layout);
        }
        //运动速度
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_move_speed = new QLabel("运动速度:");
            m_edit_move_speed = new QLineEdit(this);
            connect(m_edit_move_speed, &QLineEdit::editingFinished, this, &control_fiber_end_pane::on_move_speed_changed);
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
    	//移动到指定位置、设置为零点以及相机复位
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            m_push_button_move_to_position = new QPushButton("移动到相机位置");
            connect(m_push_button_move_to_position, &QPushButton::clicked, this, &control_fiber_end_pane::on_move_to_position);
            m_push_button_set_position_zero = new QPushButton("相机位置设为零点");
            connect(m_push_button_set_position_zero, &QPushButton::clicked, this, &control_fiber_end_pane::on_set_current_position_zero);
            m_push_button_reset_position = new QPushButton("相机复位");
            connect(m_push_button_reset_position, &QPushButton::clicked, this, &control_fiber_end_pane::on_reset_position);
            h_layout->addWidget(m_push_button_move_to_position);
            h_layout->addWidget(m_push_button_set_position_zero);
            h_layout->addWidget(m_push_button_reset_position);
            formLayout->addRow(h_layout);
        }
        //横向移动距离与纵向移动距离
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_step_x = new QLabel("移动步长(X):");
            m_edit_move_step_x = new QLineEdit(this);
            connect(m_edit_move_step_x, &QLineEdit::editingFinished, this, &control_fiber_end_pane::on_move_step_x_changed);
            QLabel* label_step_y = new QLabel("移动步长(Y):");
            m_edit_move_step_y = new QLineEdit();
            connect(m_edit_move_step_y, &QLineEdit::editingFinished, this, &control_fiber_end_pane::on_move_step_y_changed);
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
            m_push_button_calibration = new QPushButton("清晰度标定");
            connect(m_push_button_calibration, &QPushButton::clicked, this, &control_fiber_end_pane::on_calibration);
            h_layout->addWidget(m_push_button_calibration);
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
        // 拍照位置列表
        {
            m_photo_location_table = create_photo_locaion_table();
            formLayout->addRow(m_photo_location_table);
            // 按钮
            QHBoxLayout* h_layout = new QHBoxLayout();
            m_push_button_add_photo_location = new QPushButton("添加");
            connect(m_push_button_add_photo_location, &QPushButton::clicked, this, &control_fiber_end_pane::on_add_photo_location);
            m_push_button_remove_photo_location = new QPushButton("移除");
            connect(m_push_button_remove_photo_location, &QPushButton::clicked, this, &control_fiber_end_pane::on_remove_photo_location);
            h_layout->addWidget(m_push_button_add_photo_location);
            h_layout->addWidget(m_push_button_remove_photo_location);
            formLayout->addRow(h_layout);
        }
        // 影像端面个数和自动检测开关
        {
            QHBoxLayout* h_layout = new QHBoxLayout();
            QLabel* label_fiber_count = new QLabel("影像端面个数 :");
            m_edit_fiber_count = new QLineEdit();
            connect(m_edit_fiber_count, &QLineEdit::editingFinished, this, &control_fiber_end_pane::on_fiber_end_count_changed);
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
    m_position_x = root["position_x"].toInt();
    m_edit_position_x->setText(QString("%1").arg(m_position_x));
    m_position_y = root["position_y"].toInt();
    m_edit_position_y->setText(QString("%1").arg(m_position_y));
    m_move_step_x = root["move_step_x"].toInt();
    m_edit_move_step_x->setText(QString("%1").arg(m_move_step_x));
    m_move_step_y = root["move_step_y"].toInt();
    m_edit_move_step_y->setText(QString("%1").arg(m_move_step_y));

    
    QJsonArray posArray = root["m_photo_location_list"].toArray();
    std::vector<st_position> photo_locations;   //拍照位，按照Y值从小到大排列
    for (int i = 0; i < posArray.size(); i++)
    {
        QJsonObject obj = posArray[i].toObject();
        int x = obj["x"].toInt();
        int y = obj["y"].toInt();
        photo_locations.emplace_back(st_position(x, y));
    }
    std::sort(photo_locations.begin(), photo_locations.end());
    m_photo_location_table->setRowCount(0);     // 清空原有数据
    m_photo_location_table->setRowCount(static_cast<int>(photo_locations.size()));    
    for (int i = 0; i < photo_locations.size(); i++)
	{
        QTableWidgetItem* xItem = new QTableWidgetItem(QString::number(photo_locations[i].m_x));
        QTableWidgetItem* yItem = new QTableWidgetItem(QString::number(photo_locations[i].m_y));
        xItem->setFlags(xItem->flags() & ~Qt::ItemIsEditable);
        yItem->setFlags(yItem->flags() & ~Qt::ItemIsEditable);
        m_photo_location_table->setItem(i, 0, xItem);
        m_photo_location_table->setItem(i, 1, yItem);
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

void control_fiber_end_pane::on_motion_parameter_changed_success(const QJsonObject& obj)
{
    QString name = obj["name"].toString();
    if(name == "set_zero" || name == "reset_position")      //设置零点或者复位，都需要更新界面上的位置信息
    {
        m_position_x = obj["x"].toInt();
        m_edit_position_x->setText(QString("%1").arg(m_position_x));
        m_position_y = obj["y"].toInt();
        m_edit_position_y->setText(QString("%1").arg(m_position_y));
    }
}

void control_fiber_end_pane::update_motion_position(int pos_x, int pos_y)
{
    m_position_x = pos_x;
    m_edit_position_x->setText(QString("%1").arg(m_position_x));
    m_position_y = pos_y;
    m_edit_position_y->setText(QString("%1").arg(m_position_y));
}

QPushButton* control_fiber_end_pane::create_push_button(const QSize& button_size, const QIcon& icon)
{
    QPushButton* button = new QPushButton();
    button->setFixedSize(button_size);
    button->setIcon(icon);
    button->setIconSize(button_size);
    return button;
}

QTableWidget* control_fiber_end_pane::create_photo_locaion_table()
{
    // 创建表格对象
    QTableWidget* table = new QTableWidget(this);
    table->setColumnCount(2);  // 两列
    table->setHorizontalHeaderLabels(QStringList() << "X坐标" << "Y坐标");
    
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);      // 表头自适应
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);              //禁止编辑修改
    table->setSelectionBehavior(QAbstractItemView::SelectRows);                 //整行选中
    table->setDragDropMode(QAbstractItemView::NoDragDrop);
    table->setWindowTitle("拍照位置列表");
    return table;
}

void control_fiber_end_pane::on_light_brightness_changed()
{
    int brightness = m_edit_light_brightness->text().toInt();
    if(brightness <= 0)
    {
        QMessageBox::information(this, QString::fromStdString("提示"), QString::fromStdString("光源亮度必须为大于0的整数!"), QMessageBox::Ok);
        return;
    }
    if (m_light_brightness == brightness)
    {
        return;
    }
    m_light_brightness = brightness;
    QJsonObject obj;
    obj["name"] = QString("set_light_brightness");
    obj["value"] = m_light_brightness;
    emit post_set_motion_parameter(obj);
}

void control_fiber_end_pane::on_move_speed_changed()
{
    int move_speed = m_edit_move_speed->text().toInt();
    if (move_speed <= 0)
    {
        QMessageBox::information(this, QString::fromStdString("提示"), QString::fromStdString("运动速度必须为大于0的整数!"), QMessageBox::Ok);
        return;
    }
    if (m_move_peed == move_speed)
    {
        return;
    }
    m_move_peed = move_speed;
    QJsonObject obj;
    obj["name"] = QString("set_move_speed");
    obj["value"] = m_move_peed;
    emit post_set_motion_parameter(obj);
}

void control_fiber_end_pane::on_move_step_x_changed()
{
    int move_step_x = m_edit_move_step_x->text().toInt();
    if (move_step_x <= 0)
    {
        QMessageBox::information(this, QString::fromStdString("提示"), QString::fromStdString("运动步长必须为大于0的整数!"), QMessageBox::Ok);
        return;
    }
    if (m_move_step_x == move_step_x)
    {
        return;
    }
    m_move_step_x = move_step_x;
    QJsonObject obj;
    obj["name"] = QString("set_move_step_x");
    obj["value"] = m_move_step_x;
    emit post_set_motion_parameter(obj);
}

void control_fiber_end_pane::on_move_step_y_changed()
{
    int move_step_y = m_edit_move_step_y->text().toInt();
    if (move_step_y <= 0)
    {
        QMessageBox::information(this, QString::fromStdString("提示"), QString::fromStdString("运动步长必须为大于0的整数!"), QMessageBox::Ok);
        return;
    }
    if (m_move_step_y == move_step_y)
    {
        return;
    }
    m_move_step_y = move_step_y;
    QJsonObject obj;
    obj["name"] = QString("set_move_step_y");
    obj["value"] = m_move_step_y;
    emit post_set_motion_parameter(obj);
}

void control_fiber_end_pane::on_set_current_position_zero()
{
    QJsonObject obj;
    obj["name"] = QString("set_zero");
    emit post_set_motion_parameter(obj);
}

void control_fiber_end_pane::on_reset_position()
{
    QJsonObject obj;
    obj["name"] = QString("reset_position");
    emit post_set_motion_parameter(obj);
}

void control_fiber_end_pane::on_move_to_position()
{
	//检查编辑框位置是否与当前位置是否相同
    int pos_x = m_edit_position_x->text().toInt();
    int pos_y = m_edit_position_y->text().toInt();
    if(m_position_x == pos_x && m_position_y == pos_y)
    {
	    return;
    }
    m_position_x = pos_x;
    m_position_y = pos_y;
    QJsonObject obj;
    obj["name"] = QString("move_to_position");
    obj["x"] = m_position_x;
    obj["y"] = m_position_y;
    emit post_move_camera(obj);
}

void control_fiber_end_pane::on_move_forward_y()
{
    QJsonObject obj;
    obj["name"] = QString("move_forward_y");
    emit post_move_camera(obj);
}

void control_fiber_end_pane::on_move_back_x()
{
    QJsonObject obj;
    obj["name"] = QString("move_back_x");
    emit post_move_camera(obj);
}

void control_fiber_end_pane::on_move_forward_x()
{
    QJsonObject obj;
    obj["name"] = QString("move_forward_x");
    emit post_move_camera(obj);
}

void control_fiber_end_pane::on_move_back_y()
{
    QJsonObject obj;
    obj["name"] = QString("move_back_y");
    emit post_move_camera(obj);
}

void control_fiber_end_pane::on_auto_focus()
{
    emit post_auto_focus();
}

void control_fiber_end_pane::on_calibration()
{
    emit post_calibration();
}

void control_fiber_end_pane::on_add_photo_location()
{
    // 将当前位置(m_position_x,m_position_y)加入到拍照位，更新服务器参数
    int cur_row(-1);     //在当前行后面插入一行记录拍照位(m_position_x,m_position_y)
    for (int i = 0;i < m_photo_location_table->rowCount();i++)
    {
        int pos_y = m_photo_location_table->item(i, 1)->text().toInt();
        if(pos_y < m_position_y)
        {
            cur_row++;
	        continue;
        }
        else if(pos_y == m_position_y)  //如果已存在，更新 X
        {
            cur_row = i;
            m_photo_location_table->item(i, 0)->setText(QString::number(m_position_x));
            break;
        }
        else
        {
	        //在 cur_row 后面插入一行
            cur_row += 1;
            m_photo_location_table->insertRow(cur_row);
            QTableWidgetItem* xItem = new QTableWidgetItem(QString::number(m_position_x));
            QTableWidgetItem* yItem = new QTableWidgetItem(QString::number(m_position_y));
            xItem->setFlags(xItem->flags() & ~Qt::ItemIsEditable);
            yItem->setFlags(yItem->flags() & ~Qt::ItemIsEditable);
            m_photo_location_table->setItem(cur_row, 0, xItem);
            m_photo_location_table->setItem(cur_row, 1, yItem);
            break;
        }
    }
    // 插入到第一行
    if(cur_row == -1)
    {
        m_photo_location_table->insertRow(0);
        QTableWidgetItem* xItem = new QTableWidgetItem(QString::number(m_position_x));
        QTableWidgetItem* yItem = new QTableWidgetItem(QString::number(m_position_y));
        xItem->setFlags(xItem->flags() & ~Qt::ItemIsEditable);
        yItem->setFlags(yItem->flags() & ~Qt::ItemIsEditable);
        m_photo_location_table->setItem(0, 0, xItem);
        m_photo_location_table->setItem(0, 1, yItem);
    }

    update_photo_location_to_server();
    
}

void control_fiber_end_pane::on_remove_photo_location()
{
    //从控件中移除选中项
    for (int i = m_photo_location_table->rowCount() - 1; i >= 0; i--)
    {
	    if(m_photo_location_table->item(i,0)->isSelected())
	    {
            m_photo_location_table->removeRow(i);
	    }
    }
    // 更新服务器参数
    update_photo_location_to_server();
}


void control_fiber_end_pane::update_photo_location_to_server()
{
    // 获取位置列表，更新服务器参数
    std::vector<st_position> positions;
    for (int i = 0; i < m_photo_location_table->rowCount(); i++)
    {
        positions.emplace_back(st_position(m_photo_location_table->item(i, 0)->text().toInt(),
            m_photo_location_table->item(i, 1)->text().toInt()));
    }
    QJsonObject obj;
    obj["name"] = QString("update_photo_location_list");
    QJsonArray array;
    for (int i = 0; i < positions.size(); i++)
    {
        QJsonObject obj;
        obj["x"] = positions[i].m_x;
        obj["y"] = positions[i].m_y;
        array.append(obj);
    }
    obj["photo_location_list"] = array;
    emit post_update_server_parameter(obj);
}

void control_fiber_end_pane::on_fiber_end_count_changed()
{
    int fiber_end_count = m_edit_fiber_count->text().toInt();
    if (fiber_end_count <= 0)
    {
        QMessageBox::information(this, QString::fromStdString("提示"), QString::fromStdString("端面数量必须为大于0的整数!"), QMessageBox::Ok);
        return;
    }
    if (m_fiber_end_count == fiber_end_count)
    {
        return;
    }
    m_fiber_end_count = fiber_end_count;
    QJsonObject obj;
    obj["name"] = QString("update_fiber_end_count");
    obj["fiber_end_count"] = m_fiber_end_count;
    emit post_update_server_parameter(obj);
}

void control_fiber_end_pane::on_auto_detect_set_changed(int check_state)
{
    QJsonObject obj;
    obj["name"] = QString("update_auto_detect");
    int auto_detect = check_state == 0 ? 0 : 1;
    obj["auto_detect"] = auto_detect;
    emit post_update_server_parameter(obj);
}

void control_fiber_end_pane::on_set_image_save_path()
{
    // 弹出选择目录对话框，初始路径为当前保存路径
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择保存目录"),
        QString::fromStdString(""), // 假设 m_config.m_save_path 是当前路径
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (dir.isEmpty())
    {
        return;
    }
    QJsonObject obj;
    obj["name"] = QString("update_save_path");
    obj["save_path"] = dir;
    emit post_update_server_parameter(obj);
}

void control_fiber_end_pane::on_start()
{
	
}

void control_fiber_end_pane::on_stop()
{
	
}