#include "control_camera_parameter_widget.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>

#include "../device_camera/interface_camera.h"

camera_parameter_widget::camera_parameter_widget(QWidget* parent)
    :QWidget(parent)
{

}

camera_parameter_widget::~camera_parameter_widget()
{

}

void camera_parameter_widget::initialize()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);

    //相机参数
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->setVerticalSpacing(8);
        formLayout->setHorizontalSpacing(10);

        // 采集帧率
        m_fps_spin_box = new QDoubleSpinBox();
        m_fps_spin_box->setRange(2.0, 1000000.0);
        m_fps_spin_box->setValue(2.0);
        m_fps_spin_box->setDecimals(4);
        m_fps_spin_box->setSingleStep(2.0);
        connect(m_fps_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_fps_changed);
        formLayout->addRow(QString::fromStdString("采集帧率 (fps)"), m_fps_spin_box);

        // 最大宽度
        m_max_width_spin_box = new QSpinBox();
        m_max_width_spin_box->setRange(0, 1000000);
        m_max_width_spin_box->setValue(0);
        m_max_width_spin_box->setEnabled(false);
        formLayout->addRow(QString::fromStdString("最大宽度"), m_max_width_spin_box);

        // 最大高度
        m_max_height_spin_box = new QSpinBox();
        m_max_height_spin_box->setRange(0, 1000000);
        m_max_height_spin_box->setValue(0);
        m_max_height_spin_box->setEnabled(false);
        formLayout->addRow(QString::fromStdString("最大高度"), m_max_height_spin_box);

        //起点偏移 X
        m_start_x_spin_box = new QSpinBox();
        m_start_x_spin_box->setRange(0, 1000000);
        m_start_x_spin_box->setValue(0);
        connect(m_start_x_spin_box, &QSpinBox::editingFinished, this, &camera_parameter_widget::on_start_x_changed);
        formLayout->addRow(QString::fromStdString("起点偏移 X:"), m_start_x_spin_box);

        //起点偏移 Y
        m_start_y_spin_box = new QSpinBox();
        m_start_y_spin_box->setRange(0, 1000000);
        m_start_y_spin_box->setValue(0);
        connect(m_start_y_spin_box, &QSpinBox::editingFinished, this, &camera_parameter_widget::on_start_y_changed);
        formLayout->addRow(QString::fromStdString("起点偏移 Y:"), m_start_y_spin_box);


        // 宽度
        m_width_spin_box = new QSpinBox();
        m_width_spin_box->setRange(0, 1000000);
        m_width_spin_box->setValue(0);
        connect(m_width_spin_box, &QSpinBox::editingFinished, this, &camera_parameter_widget::on_width_changed);
        formLayout->addRow(QString::fromStdString("宽度"), m_width_spin_box);

        // 高度
        m_height_spin_box = new QSpinBox();
        m_height_spin_box->setRange(0, 1000000);
        m_height_spin_box->setValue(0);
        connect(m_height_spin_box, &QSpinBox::editingFinished, this, &camera_parameter_widget::on_height_changed);
        formLayout->addRow(QString::fromStdString("高度"), m_height_spin_box);

        // 像素格式
        m_pixel_format_combo_box = new QComboBox();
        connect(m_pixel_format_combo_box, &QComboBox::currentTextChanged, this, &camera_parameter_widget::on_pixel_format_changed);
        formLayout->addRow(QString::fromStdString("像素格式"), m_pixel_format_combo_box);

        // 自动曝光
        m_auto_exposure_combo_box = new QComboBox();
        //m_auto_exposure_combo_box->addItem(QString::fromStdString("开启"));
        //m_auto_exposure_combo_box->addItem(QString::fromStdString("关闭"));
        //m_auto_exposure_combo_box->setCurrentIndex(0);
        connect(m_auto_exposure_combo_box, &QComboBox::currentTextChanged, this, &camera_parameter_widget::on_exposure_set_changed);
        formLayout->addRow(QString::fromStdString("自动曝光"), m_auto_exposure_combo_box);

        //自动曝光时间下限
        m_auto_exposure_floor_spin_box = new QDoubleSpinBox();
        m_auto_exposure_floor_spin_box->setRange(0.0, 10000000.0);
        m_auto_exposure_floor_spin_box->setValue(0.0);
        m_auto_exposure_floor_spin_box->setDecimals(4);
        connect(m_auto_exposure_floor_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_auto_exposure_floor_changed);
        formLayout->addRow(QString::fromStdString("自动曝光时间下限"), m_auto_exposure_floor_spin_box);


        //自动曝光时间上限
        m_auto_exposure_upper_spin_box = new QDoubleSpinBox();
        m_auto_exposure_upper_spin_box->setRange(0.0, 10000000.0);
        m_auto_exposure_upper_spin_box->setValue(10000000.0);
        m_auto_exposure_upper_spin_box->setDecimals(4);
        connect(m_auto_exposure_upper_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_auto_exposure_upper_changed);
        formLayout->addRow(QString::fromStdString("自动曝光时间上限"), m_auto_exposure_upper_spin_box);

        // 曝光时间
        m_exposure_time_spin_box = new QDoubleSpinBox();
        m_exposure_time_spin_box->setRange(0.0, 10000000.0);
        m_exposure_time_spin_box->setValue(0.0);
        m_exposure_time_spin_box->setDecimals(4);
        m_exposure_time_spin_box->setSingleStep(10);
        m_exposure_time_spin_box->setEnabled(false);
        connect(m_exposure_time_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_exposure_time_changed);
        formLayout->addRow(QString::fromStdString("曝光时间 (us)"), m_exposure_time_spin_box);

        // 自动增益
        m_auto_gain_combo_box = new QComboBox();
        //m_auto_gain_combo_box->addItem(QString::fromStdString("开启"));
        //m_auto_gain_combo_box->addItem(QString::fromStdString("关闭"));
        //m_auto_gain_combo_box->setCurrentIndex(0);
        connect(m_auto_gain_combo_box, &QComboBox::currentTextChanged, this, &camera_parameter_widget::on_gain_set_changed);
        formLayout->addRow(QString::fromStdString("自动增益"), m_auto_gain_combo_box);

        // 自动增益下限
        m_auto_gain_floor_spin_box = new QDoubleSpinBox();
        m_auto_gain_floor_spin_box->setRange(1.0, 16.0);
        m_auto_gain_floor_spin_box->setValue(1.0);
        m_auto_gain_floor_spin_box->setSingleStep(0.01);
        m_auto_gain_floor_spin_box->setDecimals(4);
        connect(m_auto_gain_floor_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_auto_gain_floor_changed);
        formLayout->addRow(QString::fromStdString("自动增益下限 (dB)"), m_auto_gain_floor_spin_box);

        // 自动增益上限
        m_auto_gain_upper_spin_box = new QDoubleSpinBox();
        m_auto_gain_upper_spin_box->setRange(1.0, 16.0);
        m_auto_gain_upper_spin_box->setValue(16.0);
        m_auto_gain_upper_spin_box->setSingleStep(0.01);
        m_auto_gain_upper_spin_box->setDecimals(4);
        connect(m_auto_gain_upper_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_auto_gain_upper_changed);
        formLayout->addRow(QString::fromStdString("自动增益上限 (dB)"), m_auto_gain_upper_spin_box);

        // 增益
        m_gain_spin_box = new QDoubleSpinBox();
        m_gain_spin_box->setRange(1.0, 16.0);
        m_gain_spin_box->setValue(1.0000);
        m_gain_spin_box->setSingleStep(0.01);
        m_gain_spin_box->setDecimals(4);
        m_gain_spin_box->setEnabled(false);
        connect(m_gain_spin_box, &QDoubleSpinBox::editingFinished, this, &camera_parameter_widget::on_gain_changed);
        formLayout->addRow(QString::fromStdString("增益 (dB)"), m_gain_spin_box);

        QGroupBox* param_group = new QGroupBox("相机参数", this);
        param_group->setLayout(formLayout);
        layout->addWidget(param_group);
    }

    //相机控制
    {
        QFormLayout* control_layout = new QFormLayout();
        control_layout->setVerticalSpacing(2);
        // 添加控件
        m_trigger_mode_combo_box = new QComboBox();
        connect(m_trigger_mode_combo_box, &QComboBox::currentTextChanged, this,
            &camera_parameter_widget::on_trigger_mode_changed);
        control_layout->addRow("触发模式", m_trigger_mode_combo_box);
        m_trigger_source_combo_box = new QComboBox();
        connect(m_trigger_source_combo_box, &QComboBox::currentTextChanged, this,
            &camera_parameter_widget::on_trigger_source_changed);
        control_layout->addRow("触发源", m_trigger_source_combo_box);

        m_start_capture = new QPushButton("开始采集");
        connect(m_start_capture, &QPushButton::clicked, this, &camera_parameter_widget::on_start_grab);
        m_stop_capture = new QPushButton("停止采集");
        connect(m_stop_capture, &QPushButton::clicked, this, &camera_parameter_widget::on_stop_grab);
        m_stop_capture->setEnabled(false);
        QHBoxLayout* button_layout = new QHBoxLayout;
        button_layout->addWidget(m_start_capture);
        button_layout->addWidget(m_stop_capture);
        QWidget* button_container = new QWidget;
        button_container->setLayout(button_layout);
        control_layout->addRow("", button_container);

        m_run_trigger = new QPushButton("执行");
        connect(m_run_trigger, &QPushButton::clicked, this, &camera_parameter_widget::on_trigger_once);
        control_layout->addRow("", m_run_trigger);

        QGroupBox* control_group = new QGroupBox("相机控制");
        control_group->setLayout(control_layout);
        layout->addWidget(control_group);
    }
    layout->addStretch(); // 添加弹性空间
}

void camera_parameter_widget::update_camera_parameter(const QJsonObject& obj)
{
    m_updated_from_code = true;
	QJsonObject root = obj["param"].toObject();
    /*******************************参数设置*******************************/
    //帧率
    {
        double frame_rate = root["frame_rate"].toDouble();
        m_fps_spin_box->setValue(frame_rate);
        m_fps_spin_box->setProperty("last_value", frame_rate);
        QJsonObject range = root["frame_rate_range"].toObject();
		double min = range["min"].toDouble();
		double max = range["max"].toDouble();
        m_fps_spin_box->setRange(min, max);
        m_fps_spin_box->setToolTip(QString("设置采集帧率，范围:[%1,%2]")
            .arg(QString::number(min, 'f', 4))
            .arg(QString::number(max, 'f', 4)));
    }

    //最大宽高
    {
        int max_width = root["maximum_width"].toInt();
        m_max_width_spin_box->setValue(max_width);
        int max_height = root["maximum_height"].toInt();
        m_max_height_spin_box->setValue(max_height);
    }

    //起点偏移 X
    {
        int start_x = root["start_x"].toInt();
        m_start_x_spin_box->setValue(start_x);
        m_start_x_spin_box->setProperty("last_value", start_x);
        QJsonObject range = root["start_x_range"].toObject();
		double min = range["min"].toDouble();
		double max = range["max"].toDouble();
        m_start_x_spin_box->setRange(static_cast<int>(min), static_cast<int>(max));
        m_start_x_spin_box->setToolTip(QString("设置起点，范围:[%1,%2]")
							.arg(static_cast<int>(min)).arg(static_cast<int>(max)));
    }

    //起点偏移 Y
    {
        int start_y = root["start_y"].toInt();
        m_start_y_spin_box->setValue(start_y);
        m_start_y_spin_box->setProperty("last_value", start_y);
        QJsonObject range = root["start_y_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_start_y_spin_box->setRange(static_cast<int>(min), static_cast<int>(max));
        m_start_y_spin_box->setToolTip(QString("设置起点，范围:[%1,%2]")
							.arg(static_cast<int>(min)).arg(static_cast<int>(max)));
    }

    //宽度
    {
        int width = root["width"].toInt();
        m_width_spin_box->setValue(width);
        m_width_spin_box->setProperty("last_value", width);
        QJsonObject range = root["width_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_width_spin_box->setRange(static_cast<int>(min), static_cast<int>(max));
        m_width_spin_box->setToolTip(QString("设置宽度，范围:[%1,%2]")
					.arg(static_cast<int>(min)).arg(static_cast<int>(max)));
    }

    //高度
    {
        int height = root["height"].toInt();
        m_height_spin_box->setValue(height);
        m_height_spin_box->setProperty("last_value", height);
        QJsonObject range = root["height_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_height_spin_box->setRange(static_cast<int>(min), static_cast<int>(max));
        m_height_spin_box->setToolTip(QString("设置高度，范围:[%1,%2]")
					.arg(static_cast<int>(min)).arg(static_cast<int>(max)));
    }

    //像素格式
    {
        m_pixel_format_combo_box->clear();
		QJsonArray fmtArray = root["pixel_formats"].toArray();
        for (int i = 0;i < fmtArray.size();i++)
        {
			QJsonObject object = fmtArray[i].toObject();
            m_pixel_format_combo_box->addItem(object["name"].toString());
        }
        QString pixel_format = root["current_pixel_format"].toString();
        m_pixel_format_combo_box->setCurrentText(pixel_format);
    }

    //自动曝光
    {
        m_auto_exposure_combo_box->clear();
        QJsonArray modes = root["auto_exposure_modes"].toArray();
        for (int i = 0; i < modes.size(); i++)
        {
            QJsonObject object = modes[i].toObject();
            m_auto_exposure_combo_box->addItem(object["name"].toString());
        }
        QString auto_exposure_mode = root["current_auto_exposure_mode"].toString();
        m_auto_exposure_combo_box->setCurrentText(auto_exposure_mode);
    }

    //自动曝光时间下限
    {
        double auto_exposure_floor = root["auto_exposure_time_floor"].toDouble();
        m_auto_exposure_floor_spin_box->setValue(auto_exposure_floor);
        m_auto_exposure_floor_spin_box->setProperty("last_value", auto_exposure_floor);
        QJsonObject range = root["auto_exposure_time_floor_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_auto_exposure_floor_spin_box->setRange(min, max);
        m_auto_exposure_floor_spin_box->setDisabled(m_auto_exposure_combo_box->currentText() == global_auto_exposure_closed);
        m_auto_exposure_floor_spin_box->setToolTip(QString("设置自动曝光时间下限，范围:[%1,%2]").
            arg(QString::number(min, 'f', 4)).
            arg(QString::number(max, 'f', 4)));
    }

    //自动曝光时间上限
    {
        double auto_exposure_upper = root["auto_exposure_time_upper"].toDouble();
        m_auto_exposure_upper_spin_box->setValue(auto_exposure_upper);
        m_auto_exposure_upper_spin_box->setProperty("last_value", auto_exposure_upper);
        QJsonObject range = root["auto_exposure_time_upper_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_auto_exposure_upper_spin_box->setRange(min, max);
        m_auto_exposure_upper_spin_box->setDisabled(m_auto_exposure_combo_box->currentText() == global_auto_exposure_closed);
        m_auto_exposure_upper_spin_box->setToolTip(QString("设置自动曝光时间上限，范围:[%1,%2]").
            arg(QString::number(min, 'f', 4)).
            arg(QString::number(max, 'f', 4)));
    }

    //曝光时间
    {
        double exposure_time = root["exposure_time"].toDouble();
        m_exposure_time_spin_box->setValue(exposure_time);
        m_exposure_time_spin_box->setProperty("last_value", exposure_time);
        QJsonObject range = root["exposure_time_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_exposure_time_spin_box->setRange(min, max);
        m_exposure_time_spin_box->setEnabled(m_auto_exposure_combo_box->currentText() == global_auto_exposure_closed);
        m_exposure_time_spin_box->setToolTip(QString("设置曝光时间，范围:[%1,%2]")
            .arg(QString::number(min, 'f', 4))
            .arg(QString::number(max, 'f', 4)));
    }

    //自动增益
    {
        m_auto_gain_combo_box->clear();
        QJsonArray modes = root["auto_gain_modes"].toArray();
        for (int i = 0; i < modes.size(); i++)
        {
            QJsonObject object = modes[i].toObject();
            m_auto_gain_combo_box->addItem(object["name"].toString());
        }
        QString auto_gain_mode = root["current_auto_gain_mode"].toString();
        m_auto_gain_combo_box->setCurrentText(auto_gain_mode);
    }

    //自动增益下限
    {
        double auto_gain_floor = root["auto_gain_floor"].toDouble();
        m_auto_gain_floor_spin_box->setValue(auto_gain_floor);
        m_auto_gain_floor_spin_box->setProperty("last_value", auto_gain_floor);
        QJsonObject range = root["auto_gain_floor_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_auto_gain_floor_spin_box->setRange(min, max);
        m_auto_gain_floor_spin_box->setDisabled(m_auto_gain_combo_box->currentText() == global_auto_gain_closed);
        m_auto_gain_floor_spin_box->setToolTip(QString("设置自动增益下限，范围:[%1,%2]")
            .arg(QString::number(min, 'f', 4))
            .arg(QString::number(max, 'f', 4)));
    }

    //自动增益上限
    {
        double auto_gain_upper = root["auto_gain_upper"].toDouble();
        m_auto_gain_upper_spin_box->setValue(auto_gain_upper);
        m_auto_gain_upper_spin_box->setProperty("last_value", auto_gain_upper);
        QJsonObject range = root["auto_gain_upper_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_auto_gain_upper_spin_box->setRange(min, max);
        m_auto_gain_upper_spin_box->setDisabled(m_auto_gain_combo_box->currentText() == global_auto_gain_closed);
        m_auto_gain_upper_spin_box->setToolTip(QString("设置自动增益上限，范围:[%1,%2]")
            .arg(QString::number(min, 'f', 4))
            .arg(QString::number(max, 'f', 4)));
    }

    //增益
    {
        double gain = root["gain"].toDouble();
        m_gain_spin_box->setValue(gain);
        m_gain_spin_box->setProperty("last_value", gain);
        QJsonObject range = root["gain_range"].toObject();
        double min = range["min"].toDouble();
        double max = range["max"].toDouble();
        m_gain_spin_box->setRange(min, max);
        m_gain_spin_box->setEnabled(m_auto_gain_combo_box->currentText() == global_auto_gain_closed);
        m_gain_spin_box->setToolTip(QString("设置增益，范围:[%1,%2]")
            .arg(QString::number(min, 'f', 4))
            .arg(QString::number(max, 'f', 4)));
    }

    /*******************************采集控制*******************************/
    //触发模式
    {
        m_trigger_mode_combo_box->clear();
        QJsonArray triggerModes = root["trigger_modes"].toArray();
        for (int i = 0; i < triggerModes.size();i++)
        {
            QJsonObject object = triggerModes[i].toObject();
            m_trigger_mode_combo_box->addItem(object["name"].toString());
        }
        QString trigger_mode = root["current_trigger_mode"].toString();
        m_trigger_mode_combo_box->setCurrentText(trigger_mode);
        if(m_trigger_mode_combo_box->currentText() == global_trigger_mode_continuous)
        {
			m_trigger_source_combo_box->setEnabled(false);
        }
        else
        {
            m_trigger_source_combo_box->setEnabled(true);
        }
    }
    //触发源
    {
        m_trigger_source_combo_box->clear();
        QJsonArray triggerSources = root["trigger_sources"].toArray();
        for (int i = 0; i < triggerSources.size(); i++)
        {
            QJsonObject object = triggerSources[i].toObject();
            m_trigger_source_combo_box->addItem(object["name"].toString());
		}
        QString trigger_source = root["current_trigger_source"].toString();
        m_trigger_source_combo_box->setCurrentText(trigger_source);
    }

    //相机可能处于采集状态
	bool is_grab_running = root["is_grab_running"].toBool();
    update_camera_grab_status(is_grab_running);
}

void camera_parameter_widget::reset_camera_parameter()
{
    m_updated_from_code = true;
    // 采集帧率
    m_fps_spin_box->setValue(0);
    // 最大宽度
    m_max_width_spin_box->setValue(0);
    // 最大高度
    m_max_height_spin_box->setValue(0);
    // 宽度
    m_width_spin_box->setValue(0);
    // 高度
    m_height_spin_box->setValue(0);
    // 像素格式
    m_pixel_format_combo_box->clear();
    // 自动曝光
    m_auto_exposure_combo_box->clear();
    //自动曝光时间下限
    m_auto_exposure_floor_spin_box->setValue(0);
    //自动曝光时间上限
    m_auto_exposure_upper_spin_box->setValue(0);
    // 曝光时间
    m_exposure_time_spin_box->setValue(0);
    // 自动增益
    m_auto_gain_combo_box->clear();
    // 自动增益下限
    m_auto_gain_floor_spin_box->setValue(0.0);
    // 自动增益上限
    m_auto_gain_upper_spin_box->setValue(0.0);
    // 增益
    m_gain_spin_box->setValue(0.0);
    //触发模式
    m_trigger_mode_combo_box->clear();
    //触发源
    m_trigger_source_combo_box->clear();
    m_updated_from_code = false;
}

void camera_parameter_widget::on_fps_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_fps_spin_box->value();
    double last_value = m_fps_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
	}
	QJsonObject obj;
    obj["name"] = QString("fps");
	obj["value"] = value;
	emit post_change_parameter(obj);
}

void camera_parameter_widget::on_start_x_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    int value = m_start_x_spin_box->value();
    int last_value = m_start_x_spin_box->property("last_value").toInt();
    if (value == last_value)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("start_x");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_start_y_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    int value = m_start_y_spin_box->value();
    int last_value = m_start_y_spin_box->property("last_value").toInt();
    if (value == last_value)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("start_y");
    obj["value"] = value;
    emit post_change_parameter(obj);

}

void camera_parameter_widget::on_width_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    int value = m_width_spin_box->value();
    int last_value = m_width_spin_box->property("last_value").toInt();
    if (value == last_value)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("width");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_height_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    int value = m_height_spin_box->value();
    int last_value = m_height_spin_box->property("last_value").toInt();
    if (value == last_value)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("height");
    obj["value"] = value;
    emit post_change_parameter(obj);
    
}

void camera_parameter_widget::on_pixel_format_changed(const QString& text)
{
    if (m_updated_from_code)
    {
        return;
    }
    QJsonObject obj;
    obj["name"] = QString("pixel_format");
    obj["value"] = text;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_exposure_set_changed(const QString& text)
{
    if (m_updated_from_code)
    {
        return;
    }
    QJsonObject obj;
    obj["name"] = QString("auto_exposure_mode");
    obj["value"] = text;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_auto_exposure_floor_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_auto_exposure_floor_spin_box->value();
    double last_value = m_auto_exposure_floor_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("auto_exposure_time_floor");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_auto_exposure_upper_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_auto_exposure_upper_spin_box->value();
    double last_value = m_auto_exposure_upper_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("auto_exposure_time_upper");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_exposure_time_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_exposure_time_spin_box->value();
    double last_value = m_exposure_time_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("exposure_time");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_gain_set_changed(const QString& text)
{
    if (m_updated_from_code)
    {
        return;
    }
    QJsonObject obj;
    obj["name"] = QString("auto_gain_mode");
    obj["value"] = text;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_auto_gain_floor_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_auto_gain_floor_spin_box->value();
    double last_value = m_auto_gain_floor_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("auto_gain_floor");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_auto_gain_upper_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_auto_gain_upper_spin_box->value();
    double last_value = m_auto_gain_upper_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("auto_gain_upper");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_gain_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    double value = m_gain_spin_box->value();
    double last_value = m_gain_spin_box->property("last_value").toDouble();
    if (fabs(last_value - value) < 0.001)
    {
        return; //如果值没有变化，则不发送消息
    }
    QJsonObject obj;
    obj["name"] = QString("gain");
    obj["value"] = value;
    emit post_change_parameter(obj);
}

/**********************************************************************************/
void camera_parameter_widget::update_camera_grab_status(const QJsonObject& obj)
{
    bool start = obj["param"].toBool();
    update_camera_grab_status(start);
}

void camera_parameter_widget::update_camera_grab_status(bool is_grab_running)
{
    m_updated_from_code = true;
    //采集中,禁用所有参数设置控件，并视情况禁用采集功能控件
    if (is_grab_running)
    {
        //禁用所有参数控件
        m_fps_spin_box->setEnabled(false);
        m_start_x_spin_box->setEnabled(false);
        m_start_y_spin_box->setEnabled(false);
        m_width_spin_box->setEnabled(false);
        m_height_spin_box->setEnabled(false);
        m_pixel_format_combo_box->setEnabled(false);
        m_auto_exposure_combo_box->setEnabled(false);
        m_auto_exposure_floor_spin_box->setEnabled(false);
        m_auto_exposure_upper_spin_box->setEnabled(false);
        m_exposure_time_spin_box->setEnabled(false);
        m_auto_gain_combo_box->setEnabled(false);
        m_auto_gain_floor_spin_box->setEnabled(false);
        m_auto_gain_upper_spin_box->setEnabled(false);
        m_gain_spin_box->setEnabled(false);
        m_trigger_mode_combo_box->setEnabled(false);
        m_trigger_source_combo_box->setEnabled(false);
        //设置按钮可用状态
        m_start_capture->setEnabled(false);
        m_stop_capture->setEnabled(true);
        if (m_trigger_mode_combo_box->currentText() == global_trigger_mode_once &&
            m_trigger_source_combo_box->currentText() == global_trigger_source_software)
        {
            m_run_trigger->setEnabled(true);
        }
        else
        {
            m_run_trigger->setEnabled(false);
        }
    }
    else
    {
        //恢复所有控件可用状态
        m_fps_spin_box->setEnabled(true);
        m_start_x_spin_box->setEnabled(true);
        m_start_y_spin_box->setEnabled(true);
        m_width_spin_box->setEnabled(true);
        m_height_spin_box->setEnabled(true);
        m_pixel_format_combo_box->setEnabled(true);
        m_auto_exposure_combo_box->setEnabled(true);
        if (m_auto_exposure_combo_box->currentText() == global_auto_exposure_closed)
        {
            m_exposure_time_spin_box->setEnabled(true);
            m_auto_exposure_floor_spin_box->setEnabled(false);
            m_auto_exposure_upper_spin_box->setEnabled(false);
        }
        else
        {
            m_exposure_time_spin_box->setEnabled(false);
            m_auto_exposure_floor_spin_box->setEnabled(true);
            m_auto_exposure_upper_spin_box->setEnabled(true);
        }
        m_auto_gain_combo_box->setEnabled(true);
        if (m_auto_gain_combo_box->currentText() == global_auto_gain_closed)
        {
            m_gain_spin_box->setEnabled(true);
            m_auto_gain_floor_spin_box->setEnabled(false);
            m_auto_gain_upper_spin_box->setEnabled(false);
        }
        else
        {
            m_gain_spin_box->setEnabled(false);
            m_auto_gain_floor_spin_box->setEnabled(true);
            m_auto_gain_upper_spin_box->setEnabled(true);
        }

        m_trigger_mode_combo_box->setEnabled(true);
        if (m_trigger_mode_combo_box->currentText() == global_trigger_mode_continuous)
        {
            m_trigger_source_combo_box->setEnabled(false);
        }
        else
        {
            m_trigger_source_combo_box->setEnabled(true);
        }
        m_start_capture->setEnabled(true);
        m_stop_capture->setEnabled(false);
        m_run_trigger->setEnabled(false);
    }
    m_updated_from_code = false;
}

void camera_parameter_widget::on_trigger_mode_changed(const QString& text)
{
    if (m_updated_from_code)
    {
        return;
    }
    QJsonObject obj;
    obj["name"] = QString("trigger_mode");
    obj["value"] = text;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_trigger_source_changed(const QString& text)
{
    if (m_updated_from_code)
    {
        return;
    }
    QJsonObject obj;
    obj["name"] = QString("trigger_source");
    obj["value"] = text;
    emit post_change_parameter(obj);
}

void camera_parameter_widget::on_start_grab()
{
    //开始采集
    emit post_start_grab(true);
}

void camera_parameter_widget::on_stop_grab()
{
    //停止采集
    emit post_start_grab(false);
}

void camera_parameter_widget::on_trigger_once()
{
	emit post_trigger_once();
}


