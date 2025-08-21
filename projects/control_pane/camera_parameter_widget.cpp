#include "camera_parameter_widget.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>


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
        connect(m_pixel_format_combo_box, &QComboBox::currentIndexChanged, this, &camera_parameter_widget::on_pixel_format_changed);
        formLayout->addRow(QString::fromStdString("像素格式"), m_pixel_format_combo_box);

        // 自动曝光
        m_auto_exposure_combo_box = new QComboBox();
        //m_auto_exposure_combo_box->addItem(QString::fromStdString("开启"));
        //m_auto_exposure_combo_box->addItem(QString::fromStdString("关闭"));
        //m_auto_exposure_combo_box->setCurrentIndex(0);
        connect(m_auto_exposure_combo_box, &QComboBox::currentIndexChanged, this, &camera_parameter_widget::on_exposure_set_changed);
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
        connect(m_auto_gain_combo_box, &QComboBox::currentIndexChanged, this, &camera_parameter_widget::on_gain_set_changed);
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
        connect(m_trigger_mode_combo_box, &QComboBox::currentIndexChanged, this,
            &camera_parameter_widget::on_trigger_mode_changed);
        control_layout->addRow("触发模式", m_trigger_mode_combo_box);
        m_trigger_source_combo_box = new QComboBox();
        connect(m_trigger_source_combo_box, &QComboBox::currentIndexChanged, this,
            &camera_parameter_widget::on_trigger_source_changed);
        control_layout->addRow("触发源", m_trigger_source_combo_box);

        m_start_capture = new QPushButton("开始采集");
        connect(m_start_capture, &QPushButton::clicked, this,&camera_parameter_widget::on_start_grab);
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
        connect(m_run_trigger, &QPushButton::clicked, this, &camera_parameter_widget::on_trigger);
        control_layout->addRow("", m_run_trigger);

        QGroupBox* control_group = new QGroupBox("相机控制", this);
        control_group->setLayout(control_layout);
        layout->addWidget(control_group);
    }

    
    layout->addStretch(); // 添加弹性空间
}

void camera_parameter_widget::set_camera(const QVariant& data)
{
    m_updated_from_code = true;
    m_camera = static_cast<interface_camera*>(data.value<void*>());
    if (m_camera == nullptr)
    {
        reset_parameters();
        m_updated_from_code = false;
        return;
    }
    /*******************************参数设置*******************************/
    //帧率
    {
        
        double frame_rate = m_camera->get_frame_rate();
        m_fps_spin_box->setValue(frame_rate);
        st_range range = m_camera->get_frame_rate_range();
        m_fps_spin_box->setRange(range.min, range.max);
        m_fps_spin_box->setToolTip(QString("设置采集帧率，范围:[%1,%2]")
            .arg(QString::number(range.min, 'f', 4))
            .arg(QString::number(range.max, 'f', 4)));
    }

	//最大宽高
    {

        int max_width = m_camera->get_maximum_width();
        m_max_width_spin_box->setValue(max_width);
        int max_height = m_camera->get_maximum_height();
        m_max_height_spin_box->setValue(max_height);
    }

	//起点偏移 X
    {
        int start_x = m_camera->get_start_x();
        m_start_x_spin_box->setValue(start_x);
        st_range range = m_camera->get_start_x_range();
        m_start_x_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_start_x_spin_box->setToolTip(QString("设置起点，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }

	//起点偏移 Y
    {
        int start_y = m_camera->get_start_y();
        m_start_y_spin_box->setValue(start_y);
        st_range range = m_camera->get_start_y_range();
        m_start_y_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_start_y_spin_box->setToolTip(QString("设置起点，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }

	//宽度
    {
        int width = m_camera->get_width();
        m_width_spin_box->setValue(width);
        st_range range = m_camera->get_width_range();
        m_width_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_width_spin_box->setToolTip(QString("设置宽度，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }

	//高度
    {
        int height = m_camera->get_height();
        m_height_spin_box->setValue(height);
        st_range range = m_camera->get_height_range();
        m_height_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_height_spin_box->setToolTip(QString("设置高度，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }

	//像素格式
    {
        m_pixel_format_combo_box->clear();
        const QMap<QString, unsigned int>& supported_formats = m_camera->enum_pixel_format();
        if (supported_formats.size() > 0)
        {
            for (QMap<QString, unsigned int>::const_iterator iter = supported_formats.begin(); iter != supported_formats.end(); ++iter)
            {
                m_pixel_format_combo_box->addItem(iter.key());
            }
        }
        QString pixel_format = m_camera->get_pixel_format();
        m_pixel_format_combo_box->setCurrentText(pixel_format);
    }

	//自动曝光
    {
        m_auto_exposure_combo_box->clear();
        const QMap<QString, unsigned int>& supported_auto_exposure_mode = m_camera->enum_supported_auto_exposure_mode();
        if (supported_auto_exposure_mode.size() > 0)
        {
            for (QMap<QString, unsigned int>::const_iterator iter = supported_auto_exposure_mode.begin(); iter != supported_auto_exposure_mode.end(); ++iter)
            {
                m_auto_exposure_combo_box->addItem(iter.key());
            }
        }
        QString auto_exposure_mode = m_camera->get_auto_exposure_mode();
        m_auto_exposure_combo_box->setCurrentText(auto_exposure_mode);
    }

	//自动曝光时间下限
    {
        double auto_exposure_floor = m_camera->get_auto_exposure_time_floor();
        m_auto_exposure_floor_spin_box->setValue(auto_exposure_floor);
        st_range range = m_camera->get_auto_exposure_time_floor_range();
        m_auto_exposure_floor_spin_box->setRange(range.min, range.max);
        m_auto_exposure_floor_spin_box->setDisabled(m_camera->is_auto_exposure_closed());
        m_auto_exposure_floor_spin_box->setToolTip(QString("设置自动曝光时间下限，范围:[%1,%2]").
            arg(QString::number(range.min, 'f', 4)).
            arg(QString::number(range.max, 'f', 4)));
    }

	//自动曝光时间上限
    {
        double auto_exposure_upper = m_camera->get_auto_exposure_time_upper();
        m_auto_exposure_upper_spin_box->setValue(auto_exposure_upper);
        st_range range = m_camera->get_auto_exposure_time_upper_range();
        m_auto_exposure_upper_spin_box->setRange(range.min, range.max);
        m_auto_exposure_upper_spin_box->setDisabled(m_camera->is_auto_exposure_closed());
        m_auto_exposure_upper_spin_box->setToolTip(QString("设置自动曝光时间上限，范围:[%1,%2]").
            arg(QString::number(range.min, 'f', 4)).
            arg(QString::number(range.max, 'f', 4)));
    }

	//曝光时间
    {
        double exposure_time = m_camera->get_exposure_time();
        m_exposure_time_spin_box->setValue(exposure_time);
        st_range range = m_camera->get_exposure_time_range();
        m_exposure_time_spin_box->setRange(range.min, range.max);
        m_exposure_time_spin_box->setEnabled(m_camera->is_auto_exposure_closed());
        m_exposure_time_spin_box->setToolTip(QString("设置曝光时间，范围:[%1,%2]")
            .arg(QString::number(range.min, 'f', 4))
            .arg(QString::number(range.max, 'f', 4)));
    }

	//自动增益
    {
        m_auto_gain_combo_box->clear();
        const QMap<QString, unsigned int>& supported_auto_gain_mode = m_camera->enum_supported_auto_gain_mode();
        if (supported_auto_gain_mode.size() > 0)
        {
            for (QMap<QString, unsigned int>::const_iterator iter = supported_auto_gain_mode.begin(); iter != supported_auto_gain_mode.end(); ++iter)
            {
                m_auto_gain_combo_box->addItem(iter.key());
            }
        }
        QString auto_gain_mode = m_camera->get_auto_gain_mode();
        m_auto_gain_combo_box->setCurrentText(auto_gain_mode);
    }

	//自动增益下限
    {
        double auto_gain_floor = m_camera->get_auto_gain_floor();
        m_auto_gain_floor_spin_box->setValue(auto_gain_floor);
        st_range range = m_camera->get_auto_gain_floor_range();
        m_auto_gain_floor_spin_box->setRange(range.min, range.max);
        m_auto_gain_floor_spin_box->setDisabled(m_camera->is_auto_gain_closed());
        m_auto_gain_floor_spin_box->setToolTip(QString("设置自动增益下限，范围:[%1,%2]")
            .arg(QString::number(range.min, 'f', 4))
            .arg(QString::number(range.max, 'f', 4)));
    }

	//自动增益上限
    {
        double auto_gain_upper = m_camera->get_auto_gain_upper();
        m_auto_gain_upper_spin_box->setValue(auto_gain_upper);
        st_range range = m_camera->get_auto_gain_upper_range();
        m_auto_gain_upper_spin_box->setRange(range.min, range.max);
        m_auto_gain_upper_spin_box->setDisabled(m_camera->is_auto_gain_closed());
        m_auto_gain_upper_spin_box->setToolTip(QString("设置自动增益上限，范围:[%1,%2]")
            .arg(QString::number(range.min, 'f', 4))
            .arg(QString::number(range.max, 'f', 4)));
    }

	//增益
    {
        double gain = m_camera->get_gain();
        m_gain_spin_box->setValue(gain);
        st_range range = m_camera->get_gain_range();
        m_gain_spin_box->setRange(range.min, range.max);
        m_gain_spin_box->setEnabled(m_camera->is_auto_gain_closed());
        m_gain_spin_box->setToolTip(QString("设置增益，范围:[%1,%2]")
            .arg(QString::number(range.min, 'f', 4))
            .arg(QString::number(range.max, 'f', 4)));
    }
    
    /*******************************采集控制*******************************/
    //触发模式
    {
        m_trigger_mode_combo_box->clear();
        const QMap<QString, unsigned int>& supported_trigger_mode = m_camera->enum_supported_trigger_mode();
        if (supported_trigger_mode.size() > 0)
        {
            for (QMap<QString, unsigned int>::const_iterator iter = supported_trigger_mode.begin(); iter != supported_trigger_mode.end(); ++iter)
            {
                m_trigger_mode_combo_box->addItem(iter.key());
            }
        }
        QString trigger_mode = m_camera->get_trigger_mode();
        m_trigger_mode_combo_box->setCurrentText(trigger_mode);
    }
    //触发源
    {
        m_trigger_source_combo_box->clear();
        const QMap<QString, unsigned int>& supported_trigger_source = m_camera->enum_supported_trigger_source();
        if (supported_trigger_source.size() > 0)
        {
            for (QMap<QString, unsigned int>::const_iterator iter = supported_trigger_source.begin(); iter != supported_trigger_source.end(); ++iter)
            {
                m_trigger_source_combo_box->addItem(iter.key());
            }
        }
        QString trigger_source = m_camera->get_trigger_source();
        m_trigger_source_combo_box->setCurrentText(trigger_source);
    }
    set_controls_status();
    m_updated_from_code = false;
}

void camera_parameter_widget::reset_parameters() const
{
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
}

void camera_parameter_widget::on_fps_changed()
{
    if(m_updated_from_code)
    {
	    return;
    }
	if (m_camera != nullptr)
	{
        int ret = m_camera->set_frame_rate(m_fps_spin_box->value());
		if(ret != STATUS_SUCCESS)
		{
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置采集帧率失败: %1").arg(ret));
		}
	}
	
}

void camera_parameter_widget::on_start_x_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_start_x(m_start_x_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置 X 方向起点失败: %1").arg(ret));
        }
        //起点变化之后需要修改宽度可设置的范围
        st_range range = m_camera->get_width_range();
        m_width_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_width_spin_box->setToolTip(QString("设置宽度，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }
}

void camera_parameter_widget::on_start_y_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_start_y(m_start_y_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置 Y 方向起点失败: %1").arg(ret));
        }
        st_range range = m_camera->get_height_range();
        m_height_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_height_spin_box->setToolTip(QString("设置高度，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }
    
}

void camera_parameter_widget::on_width_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_width(m_width_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置宽度失败: %1").arg(ret));
        }
        //宽度变化之后需要修改起点可设置的范围
        st_range range = m_camera->get_start_x_range();
        m_start_x_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_start_x_spin_box->setToolTip(QString("设置起点，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }
}

void camera_parameter_widget::on_height_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_height(m_height_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置高度失败: %1").arg(ret));
        }
        //高度变化之后需要修改起点可设置的范围
        st_range range = m_camera->get_start_y_range();
        m_start_y_spin_box->setRange(static_cast<int>(range.min), static_cast<int>(range.max));
        m_start_y_spin_box->setToolTip(QString("设置起点，范围:[%1,%2]").arg(static_cast<int>(range.min)).arg(static_cast<int>(range.max)));
    }
}

void camera_parameter_widget::on_pixel_format_changed(int index)
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_pixel_format(m_pixel_format_combo_box->currentText());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置像素格式失败: %1").arg(ret));
        }
    }
}

void camera_parameter_widget::on_exposure_set_changed(int index)
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        QString current_text = m_auto_exposure_combo_box->currentText();
        int ret = m_camera->set_auto_exposure_mode(current_text);
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置曝光模式失败: %1").arg(ret));
        }
        if (m_auto_exposure_floor_spin_box != nullptr)
        {
            m_auto_exposure_floor_spin_box->setDisabled(m_camera->is_auto_exposure_closed());
        }
        if (m_auto_exposure_upper_spin_box != nullptr)
        {
            m_auto_exposure_upper_spin_box->setDisabled(m_camera->is_auto_exposure_closed());
        }
        if (m_exposure_time_spin_box != nullptr)
        {
            m_exposure_time_spin_box->setEnabled(m_camera->is_auto_exposure_closed());
        }
    }
}

void camera_parameter_widget::on_auto_exposure_floor_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_auto_exposure_time_floor(m_auto_exposure_floor_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置自动曝光时间下限失败: %1").arg(ret));
        }
        //自动曝光时间下限变化之后需要修改对应上限可设置的范围
        st_range range = m_camera->get_auto_exposure_time_upper_range();
        m_auto_exposure_upper_spin_box->setRange(range.min, range.max);
        m_auto_exposure_upper_spin_box->setToolTip(QString("设置自动曝光时间上限，范围:[%1,%2]").arg(range.min).arg(range.max));
    }
}

void camera_parameter_widget::on_auto_exposure_upper_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_auto_exposure_time_upper(m_auto_exposure_upper_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置自动曝光时间上限失败: %1").arg(ret));
        }
        //自动曝光时间上限变化之后需要修改对应下限可设置的范围
        st_range range = m_camera->get_auto_exposure_time_floor_range();
        m_auto_exposure_floor_spin_box->setRange(range.min, range.max);
        m_auto_exposure_floor_spin_box->setToolTip(QString("设置自动曝光时间上限，范围:[%1,%2]").arg(range.min).arg(range.max));
    }
}

void camera_parameter_widget::on_exposure_time_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_exposure_time(m_exposure_time_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置曝光时间失败: %1").arg(ret));
        }
    }
}

void camera_parameter_widget::on_gain_set_changed(int index)
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        QString current_text = m_auto_gain_combo_box->currentText();
        int ret = m_camera->set_auto_gain_mode(current_text);
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置增益模式失败: %1").arg(ret));
        }
        if (m_auto_gain_floor_spin_box != nullptr)
        {
            m_auto_gain_floor_spin_box->setDisabled(m_camera->is_auto_gain_closed());
        }
        if (m_auto_gain_upper_spin_box != nullptr)
        {
            m_auto_gain_upper_spin_box->setDisabled(m_camera->is_auto_gain_closed());
        }
        if (m_gain_spin_box != nullptr)
        {
            m_gain_spin_box->setEnabled(m_camera->is_auto_gain_closed());
        }
    }
}

void camera_parameter_widget::on_auto_gain_floor_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_auto_gain_floor(m_auto_gain_floor_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置自动增益下限失败: %1").arg(ret));
        }
    }
}

void camera_parameter_widget::on_auto_gain_upper_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_auto_gain_upper(m_auto_gain_upper_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置自动增益上限失败: %1").arg(ret));
        }
    }
}

void camera_parameter_widget::on_gain_changed()
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        int ret = m_camera->set_gain(m_gain_spin_box->value());
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), QString("设置增益失败: %1").arg(ret));
        }
    }
}

/**********************************************************************************/
void camera_parameter_widget::on_trigger_mode_changed(int index)
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        QString current_text = m_trigger_mode_combo_box->currentText();
        int ret = m_camera->set_trigger_mode(current_text);
        if(ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"), 
                QString("设置触发模式失败: %1").arg(ret));
            return;
        }
        set_controls_status();
    }
	
}

void camera_parameter_widget::on_trigger_source_changed(int index)
{
    if (m_updated_from_code)
    {
        return;
    }
    if (m_camera != nullptr)
    {
        QString current_text = m_trigger_source_combo_box->currentText();
        int ret = m_camera->set_trigger_source(current_text);
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"),
                QString("设置触发源失败: %1").arg(ret));
            return;
        }
        //触发源为软触发时触发按钮才可用
        set_controls_status();
    }
}

void camera_parameter_widget::on_start_grab()
{
    //开始采集
    if (m_camera != nullptr)
    {
        int ret = m_camera->start_grab();
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"),
                QString("开始采集失败: %1").arg(ret), QMessageBox::Ok);
            return;
        }
    }
    set_controls_status();
}

void camera_parameter_widget::on_trigger()
{
    if (m_camera != nullptr)
    {
        QImage image = m_camera->trigger_once();
        emit post_trigger_finished(QVariant::fromValue(image));
    }
}

void camera_parameter_widget::on_stop_grab()
{
    //停止采集
    if(m_camera != nullptr)
    {
        int ret = m_camera->stop_grab();
        if (ret != STATUS_SUCCESS)
        {
            QMessageBox::information(this, QString::fromStdString("错误"),
                QString("停止采集失败: %1").arg(ret), QMessageBox::Ok);
            return;
        }
    }
    set_controls_status();
}

void camera_parameter_widget::set_controls_status()
{
    m_trigger_mode_combo_box->setEnabled(true);
    m_trigger_source_combo_box->setEnabled(true);
    m_start_capture->setEnabled(true);
    m_run_trigger->setEnabled(true);
    m_stop_capture->setEnabled(true);
    if(m_trigger_mode_combo_box->currentText() == global_trigger_mode_continuous)
    {
        m_trigger_source_combo_box->setEnabled(false);
    }
    if (m_trigger_source_combo_box->currentText() != global_trigger_source_software)
    {
        m_run_trigger->setEnabled(false);
    }
    if(m_camera->is_grab_running())
    {
        m_trigger_mode_combo_box->setEnabled(false);
        m_trigger_source_combo_box->setEnabled(false);
        m_start_capture->setEnabled(false);
    }
    else
    {
        m_stop_capture->setEnabled(false);
        m_run_trigger->setEnabled(false);
    }
}