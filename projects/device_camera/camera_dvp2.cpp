#include "camera_dvp2.h"

#include "dvpParam.h"

dvp2_camera::dvp2_camera(const QString& user_name, const QString& friendly_name, const QString& unique_id)
			:m_user_name(user_name),m_friendly_name(friendly_name)
{
	m_unique_id = unique_id;
    m_sdk_type = SDK_DVP2;
    m_map_ret_status.insert(DVP_STATUS_OK, STATUS_SUCCESS);
    m_map_ret_status.insert(DVP_STATUS_INVALID_HANDLE, STATUS_ERROR_HANDLE);
    m_map_ret_status.insert(DVP_STATUS_NOT_SUPPORTED, STATUS_ERROR_SUPPORT);
    m_map_ret_status.insert(DVP_STATUS_PARAMETER_INVALID, STATUS_ERROR_PARAMETER);

    m_global_map_auto_exposure_mode.insert(QString::fromStdString("Disable"), global_auto_exposure_closed);
    m_global_map_auto_exposure_mode.insert(QString::fromStdString("Once"), global_auto_exposure_once);
    m_global_map_auto_exposure_mode.insert(QString::fromStdString("Continuous"), global_auto_exposure_continuous);
    m_global_map_auto_exposure_mode.insert(QString::fromStdString("AE_OP_OFF"), global_auto_exposure_closed);
    m_global_map_auto_exposure_mode.insert(QString::fromStdString("AE_OP_ONCE"), global_auto_exposure_once);
    m_global_map_auto_exposure_mode.insert(QString::fromStdString("AE_OP_CONTINUOUS"), global_auto_exposure_continuous);

    m_global_map_auto_gain_mode.insert(QString::fromStdString("Disable"), global_auto_gain_closed);
    m_global_map_auto_gain_mode.insert(QString::fromStdString("Once"), global_auto_gain_once);
    m_global_map_auto_gain_mode.insert(QString::fromStdString("Continuous"), global_auto_gain_continuous);
}

dvp2_camera::~dvp2_camera()
{
    
}

bool dvp2_camera::is_device_accessible(unsigned int nAccessMode) const
{
    return true;
}

int dvp2_camera::create_device_handle()
{
    return 0;
}

int dvp2_camera::open()
{
    dvpStatus status = dvpOpenByUserId(m_user_name.toStdString().c_str(), OPEN_NORMAL, &m_device_handle);
    if(status != DVP_STATUS_OK)
    {
        status = dvpOpenByName(m_friendly_name.toStdString().c_str(), OPEN_NORMAL, &m_device_handle);
        if(status != DVP_STATUS_OK)
        {
            if (!m_map_ret_status.contains(status))
            {
                return STATUS_ERROR_UNKNOWN;
            }
            return m_map_ret_status[status];
		}
    }
    m_is_opened = true;
    return STATUS_SUCCESS;
}

int dvp2_camera::close()
{
    if(m_device_handle != 0)
    {
        dvpStreamState state;
        dvpGetStreamState(m_device_handle, &state);
        if (state == STATE_STARTED)
        {
            dvpStatus status = dvpStop(m_device_handle);
            if(status != DVP_STATUS_OK)
            {
                if (!m_map_ret_status.contains(status))
                {
                    return STATUS_ERROR_UNKNOWN;
                }
                return m_map_ret_status[status];
            }
        }

        dvpStatus status = dvpClose(m_device_handle);
        if (status != DVP_STATUS_OK)
        {
            if (!m_map_ret_status.contains(status))
            {
                return STATUS_ERROR_UNKNOWN;
            }
            return m_map_ret_status[status];
        }
		m_is_opened = false;
        m_device_handle = 0;
    }
    return STATUS_SUCCESS;
}

bool dvp2_camera::is_device_connected()
{
    return true;
}

/****************************一些基础参数获取******************************/
double dvp2_camera::get_frame_rate()
{
    dvpFrameCount frame_count;
    dvpStatus status = dvpGetFrameCount(m_device_handle, &frame_count);
    if(status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return static_cast<double>(frame_count.fFrameRate);
}

st_range dvp2_camera::get_frame_rate_range()
{
    return st_range(0,10);
}

int dvp2_camera::set_frame_rate(double frame_rate)
{
    QString str = QString("%1").arg(frame_rate);
    dvpStatus status = dvpSetConfigString(m_device_handle,"AcquisitionFrameRateValue", str.toStdString().c_str());
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

int dvp2_camera::get_maximum_width()
{
    dvpSensorInfo sensor_info;
    dvpStatus status = dvpGetSensorInfo(m_device_handle, &sensor_info);
    if(status != DVP_STATUS_OK)
    {
        return -1;
    }
    return sensor_info.region.iMaxW;
    
}

int dvp2_camera::get_maximum_height()
{
    dvpSensorInfo sensor_info;
    dvpStatus status = dvpGetSensorInfo(m_device_handle, &sensor_info);
    if (status != DVP_STATUS_OK)
    {
        return -1;
    }
    return sensor_info.region.iMaxH;
}

int dvp2_camera::get_start_x()
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        return -1;
    }
    return region.X;
}

st_range dvp2_camera::get_start_x_range()
{
    return st_range(0, get_maximum_width() - get_width());
}

int dvp2_camera::set_start_x(int start_x)
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    region.X = start_x;
	status = dvpSetRoi(m_device_handle, region);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

int dvp2_camera::get_start_y()
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        return -1;
    }
    return region.Y;
}

st_range dvp2_camera::get_start_y_range()
{
    return st_range(0, get_maximum_height() - get_height());
}

int dvp2_camera::set_start_y(int start_y)
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    region.Y = start_y;
    status = dvpSetRoi(m_device_handle, region);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

int dvp2_camera::get_width()
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        return -1;
    }
    return region.W;
}

st_range dvp2_camera::get_width_range()
{
    return st_range(0, get_maximum_width() - get_start_x());
}

int dvp2_camera::set_width(int width)
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    region.W = width;
    status = dvpSetRoi(m_device_handle, region);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

int dvp2_camera::get_height()
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        return -1;
    }
    return region.H;
}

st_range dvp2_camera::get_height_range()
{
    return st_range(0, get_maximum_height() - get_start_y());
}

int dvp2_camera::set_height(int height)
{
    dvpRegion region;
    dvpStatus status = dvpGetRoi(m_device_handle, &region);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    region.H = height;
    status = dvpSetRoi(m_device_handle, region);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

const QMap<QString, unsigned int>& dvp2_camera::enum_pixel_format()
{
    if(m_supported_formats.isEmpty())
    {
        dvpSelectionDescr des;
        dvpStatus status = dvpGetSourceFormatSelDescr(m_device_handle, &des);
        for (int i = 0;i < des.uCount;i++)
        {
            dvpFormatSelection fs;
            status = dvpGetSourceFormatSelDetail(m_device_handle, i, &fs);
            if(status == DVP_STATUS_OK)
            {
                m_supported_formats.insert(QString::fromStdString(fs.selection.string), fs.selection.iIndex);
            }
        }
    }
    return m_supported_formats;
}

QString dvp2_camera::get_pixel_format()
{
    dvpUint32 index(0);
	dvpStatus status = dvpGetSourceFormatSel(m_device_handle, &index);
    dvpFormatSelection fs;
    status = dvpGetSourceFormatSelDetail(m_device_handle, index, &fs);
    return QString::fromStdString(fs.selection.string);
}

int dvp2_camera::set_pixel_format(unsigned int format)
{
    dvpStatus status = dvpSetSourceFormatSel(m_device_handle, format);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

int dvp2_camera::set_pixel_format(const QString& sFormat)
{
    if (m_supported_formats.find(sFormat) == m_supported_formats.end())
    {
        return m_map_ret_status[STATUS_ERROR_PARAMETER];
    }
    return set_pixel_format(m_supported_formats[sFormat]);
}

const QMap<QString, unsigned int>& dvp2_camera::enum_supported_auto_exposure_mode()
{
    if (m_supported_auto_exposure_mode.isEmpty())
    {
        int cur_mode = 0;
        unsigned int supported_count = 0;
        int supported_values[64] = { 0 };
        dvpStatus  status = dvpGetEnumValue(m_device_handle, V_EXPOSURE_AUTO_E, &cur_mode, supported_values, &supported_count);
        for (int i = 0; i < (int)supported_count; i++)
        {
            dvpEnumDescr stEnumDescr;
            dvpGetEnumDescr(m_device_handle, V_EXPOSURE_AUTO_E, i, &stEnumDescr);
            QString name = QString::fromStdString(stEnumDescr.szEnumName);
            QMap<QString, QString>::iterator iter = m_global_map_auto_exposure_mode.find(name);
            if (iter != m_global_map_auto_exposure_mode.end())
            {
                m_map_auto_exposure_mode.insert(iter.key(), iter.value());
                m_supported_auto_exposure_mode.insert(m_map_auto_exposure_mode[name], supported_values[i]);
            }
            else
            {
                m_supported_auto_exposure_mode.insert(QString::fromStdString("error_mode"), supported_values[i]);
            }
        }
        if (m_supported_auto_exposure_mode.isEmpty())
        {
            m_supported_auto_exposure_mode.insert(global_auto_exposure_closed, cur_mode);
        }
    }
    return m_supported_auto_exposure_mode;
}

QString dvp2_camera::get_auto_exposure_mode()
{
    char sz_text[64] = { 0 };
    dvpStatus status = dvpGetEnumValueByString(m_device_handle, V_EXPOSURE_AUTO_E, sz_text);
    if(status != DVP_STATUS_OK)
    {
        return QString("error_mode");
    }
    QString mode = QString::fromStdString(sz_text);
    if(m_map_auto_exposure_mode.contains(mode))
    {
        return m_map_auto_exposure_mode[mode];
    }
    return QString("error_mode");
}

int dvp2_camera::set_auto_exposure_mode(QString auto_exposure_mode)
{
    //这里根据字符串设置
    QString key("");
    for (QMap<QString, QString>::iterator iter = m_map_auto_exposure_mode.begin(); iter != m_map_auto_exposure_mode.end();++iter)
    {
	    if(iter.value() == auto_exposure_mode)
	    {
            key = iter.key();
            break;
	    }
    }
    if(key.isEmpty())
    {
        return STATUS_ERROR_PARAMETER;
    }
    dvpStatus status = dvpSetEnumValueByString(m_device_handle, V_EXPOSURE_AUTO_E, key.toStdString().c_str());
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

bool dvp2_camera::is_auto_exposure_closed()
{
    return get_auto_exposure_mode() == global_auto_exposure_closed;
}

double dvp2_camera::get_auto_exposure_time_floor()
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if(status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return config.fExposureMin;
}

st_range dvp2_camera::get_auto_exposure_time_floor_range()
{
    st_range range;
    float exposure_time(0.0);
    dvpFloatDescr exposure_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_EXPOSURE_TIME_F, &exposure_time, &exposure_descr);
    if (status != DVP_STATUS_OK)
    {
        return range;
    }
    range.min = static_cast<double>(exposure_descr.fMin);
    range.max = get_auto_exposure_time_upper();
    return range;
}

int dvp2_camera::set_auto_exposure_time_floor(double exposure_time)
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if(status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    config.fExposureMin = exposure_time;
    status = dvpSetAeConfig(m_device_handle, config);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

double dvp2_camera::get_auto_exposure_time_upper()
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if (status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return config.fExposureMax;
}

st_range dvp2_camera::get_auto_exposure_time_upper_range()
{
    st_range range;
    float exposure_time(0.0);
    dvpFloatDescr exposure_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_EXPOSURE_TIME_F, &exposure_time, &exposure_descr);
    if (status != DVP_STATUS_OK)
    {
        return range;
    }
    range.min = get_auto_exposure_time_floor();
    range.max = static_cast<double>(exposure_descr.fMax);
    return range;
}

int dvp2_camera::set_auto_exposure_time_upper(double exposure_time)
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    config.fExposureMax = exposure_time;
    status = dvpSetAeConfig(m_device_handle, config);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

double dvp2_camera::get_exposure_time()
{
    float exposure_time(0.0);
    dvpFloatDescr exposure_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_EXPOSURE_TIME_F, &exposure_time, &exposure_descr);
    if (status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return static_cast<double>(exposure_time);
}

st_range dvp2_camera::get_exposure_time_range()
{
    st_range range;
    float exposure_time(0.0);
    dvpFloatDescr exposure_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_EXPOSURE_TIME_F, &exposure_time, &exposure_descr);
    if (status != DVP_STATUS_OK)
    {
        return range;
    }
    range.min = static_cast<double>(exposure_descr.fMin);
    range.max = static_cast<double>(exposure_descr.fMax);
    return range;
}

int dvp2_camera::set_exposure_time(double exposure_time)
{
    dvpStatus status = dvpSetFloatValue(m_device_handle, V_EXPOSURE_TIME_F, static_cast<float>(exposure_time));
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

const QMap<QString, unsigned int>& dvp2_camera::enum_supported_auto_gain_mode()
{
    if (m_supported_auto_gain_mode.isEmpty())
    {
        int cur_mode = 0;
        unsigned int supported_count = 0;
        int supported_values[64] = { 0 };
        dvpStatus  status = dvpGetEnumValue(m_device_handle, V_GAIN_AUTO_E, &cur_mode, supported_values, &supported_count);

        for (int i = 0; i < (int)supported_count; i++)
        {
            dvpEnumDescr stEnumDescr;
            dvpGetEnumDescr(m_device_handle, V_GAIN_AUTO_E, i, &stEnumDescr);
            QString name = QString::fromStdString(stEnumDescr.szEnumName);
            QMap<QString, QString>::iterator iter = m_global_map_auto_gain_mode.find(name);
            if (iter != m_global_map_auto_gain_mode.end())
            {
                m_map_auto_gain_mode.insert(iter.key(), iter.value());
                m_supported_auto_gain_mode.insert(m_map_auto_gain_mode[name], supported_values[i]);
            }
            else
            {
                m_supported_auto_gain_mode.insert(QString::fromStdString("error_mode"), supported_values[i]);
            }

        }
        if (m_supported_auto_gain_mode.isEmpty())
        {
            m_supported_auto_gain_mode.insert(global_auto_gain_closed, cur_mode);
        }
    }
    return m_supported_auto_gain_mode;
}

QString dvp2_camera::get_auto_gain_mode()
{
    char sz_text[64] = { 0 };
    dvpStatus status = dvpGetEnumValueByString(m_device_handle, V_GAIN_AUTO_E, sz_text);
    if (status != DVP_STATUS_OK)
    {
        return QString::fromStdString("关闭");
    }
    QString mode = QString::fromStdString(sz_text);
    if (m_map_auto_gain_mode.contains(mode))
    {
        return m_map_auto_gain_mode[mode];
    }
    return QString::fromStdString("关闭");
}

int dvp2_camera::set_auto_gain_mode(QString auto_gain_mode)
{
    QString key("");
    for (QMap<QString, QString>::iterator iter = m_map_auto_gain_mode.begin(); iter != m_map_auto_gain_mode.end(); ++iter)
    {
        if (iter.value() == auto_gain_mode)
        {
            key = iter.key();
            break;
        }
    }
    if (key.isEmpty())
    {
        return STATUS_ERROR_PARAMETER;
    }
    dvpStatus status = dvpSetEnumValueByString(m_device_handle, V_GAIN_AUTO_E, key.toStdString().c_str());
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

bool dvp2_camera::is_auto_gain_closed()
{
    return get_auto_gain_mode() == global_auto_gain_closed;
}

double dvp2_camera::get_auto_gain_floor()
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if (status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return static_cast<double>(config.fGainMin);
}

st_range dvp2_camera::get_auto_gain_floor_range()
{
    st_range range;
    float gain(0.0);
    dvpFloatDescr gain_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_GAIN_F, &gain, &gain_descr);
    if (status != DVP_STATUS_OK)
    {
        return range;
    }
    range.min = static_cast<double>(gain_descr.fMin);
    range.max = get_auto_gain_upper();
    return range;
}

int dvp2_camera::set_auto_gain_floor(double gain)
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    config.fGainMin = gain;
    status = dvpSetAeConfig(m_device_handle, config);
    //dvpAeConfig cfg;
	//status = dvpGetAeConfig(m_device_handle, &cfg);      //测试设置是否生效,这里测试有效，但是关闭相机之后再打开，读取的下限被重置成1.0
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

double dvp2_camera::get_auto_gain_upper()
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if (status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return static_cast<double>(config.fGainMax);
}

st_range dvp2_camera::get_auto_gain_upper_range()
{
    st_range range;
    float gain(0.0);
    dvpFloatDescr gain_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_GAIN_F, &gain, &gain_descr);
    if (status != DVP_STATUS_OK)
    {
        return range;
    }
    range.min = get_auto_gain_floor();
    range.max = static_cast<double>(gain_descr.fMax);
    return range;
}

int dvp2_camera::set_auto_gain_upper(double gain)
{
    dvpAeConfig config;
    dvpStatus status = dvpGetAeConfig(m_device_handle, &config);
    if (status != DVP_STATUS_OK)
    {
        if (!m_map_ret_status.contains(status))
        {
            return STATUS_ERROR_UNKNOWN;
        }
        return m_map_ret_status[status];
    }
    config.fGainMax = gain;
    status = dvpSetAeConfig(m_device_handle, config);
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

double dvp2_camera::get_gain()
{
    float gain(0.0);
    dvpFloatDescr gain_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_GAIN_F, &gain, &gain_descr);
    if (status != DVP_STATUS_OK)
    {
        return -1.0;
    }
    return static_cast<double>(gain);
}

st_range dvp2_camera::get_gain_range()
{
    st_range range;
    float gain(0.0);
    dvpFloatDescr gain_descr;
    dvpStatus status = dvpGetFloatValue(m_device_handle, V_GAIN_F, &gain, &gain_descr);
    if (status != DVP_STATUS_OK)
    {
        return range;
    }
    range.min = static_cast<double>(gain_descr.fMin);
    range.max = static_cast<double>(gain_descr.fMax);
    return range;
}

int dvp2_camera::set_gain(double gain)
{
    dvpStatus status = dvpSetFloatValue(m_device_handle, V_GAIN_F, static_cast<float>(gain));
    if (!m_map_ret_status.contains(status))
    {
        return STATUS_ERROR_UNKNOWN;
    }
    return m_map_ret_status[status];
}

/****************************采集控制******************************/
const QMap<QString, unsigned int>& dvp2_camera::enum_supported_trigger_mode()
{
    /*********************************
     * DVP2 的 SDK 通过 dvpGetTriggerState 来获取触发模式，该接口只能获取一个 bool 变量表示 连续模式(false) 和 触发模式(true)
     * 这里支持的触发模式固定为连续触发和触发一次
     *********************************/
    if (m_supported_trigger_mode.isEmpty())
    {
        m_supported_trigger_mode.insert(global_trigger_mode_continuous, 0);     //连续触发
        m_supported_trigger_mode.insert(global_trigger_mode_once, 1);           //触发一次
    }
    return m_supported_trigger_mode;
}

QString dvp2_camera::get_trigger_mode()
{
	bool trigger_state = false;     
    dvpStatus ret = dvpGetTriggerState(m_device_handle, &trigger_state);
    if(ret != DVP_STATUS_OK)
    {
        return global_trigger_mode_continuous;      //默认连续模式
	}
    if(trigger_state)
    {
        return global_trigger_mode_once;
    }
    return global_trigger_mode_continuous;
}

int dvp2_camera::set_trigger_mode(QString trigger_mode)
{
    bool trigger_state(false);
    if(trigger_mode == global_trigger_mode_once)
    {
        trigger_state = true;
    }
    dvpStatus ret = dvpSetTriggerState(m_device_handle, trigger_state);
	return map_ret_status(ret);
	
}

const QMap<QString, unsigned int>& dvp2_camera::enum_supported_trigger_source()
{
    /*********************************
     * DVP2 的 SDK 通过 dvpGetTriggerSource 来获取触发源，该接口只能获取一个枚举变量表示当前触发源
     * 这里支持的触发源设置为所有的枚举值
     *********************************/
    if (m_supported_trigger_source.isEmpty())
    {
        m_supported_trigger_source.insert(global_trigger_source_software, TRIGGER_SOURCE_SOFTWARE);
        m_supported_trigger_source.insert(global_trigger_source_line1, TRIGGER_SOURCE_LINE1);
        m_supported_trigger_source.insert(global_trigger_source_line2, TRIGGER_SOURCE_LINE2);
        m_supported_trigger_source.insert(global_trigger_source_line3, TRIGGER_SOURCE_LINE3);
        m_supported_trigger_source.insert(global_trigger_source_line4, TRIGGER_SOURCE_LINE4);
        m_supported_trigger_source.insert(global_trigger_source_line5, TRIGGER_SOURCE_LINE5);
        m_supported_trigger_source.insert(global_trigger_source_line6, TRIGGER_SOURCE_LINE6);
        m_supported_trigger_source.insert(global_trigger_source_line7, TRIGGER_SOURCE_LINE7);
        m_supported_trigger_source.insert(global_trigger_source_line8, TRIGGER_SOURCE_LINE8);
    }
    return m_supported_trigger_source;
}

QString dvp2_camera::get_trigger_source()
{
    dvpTriggerSource trigger_source;
	dvpStatus ret = dvpGetTriggerSource(m_device_handle, &trigger_source);
    if(ret != DVP_STATUS_OK)
    {
        return global_trigger_source_software;      //默认返回软触发
    }
    for (QMap<QString, unsigned int>::const_iterator iter = m_supported_trigger_source.cbegin();
        iter != m_supported_trigger_source.cend(); ++iter) 
    {
        if(iter.value() == trigger_source)
        {
            return iter.key();      //返回当前触发源
		}
    }
    return global_trigger_source_software;      //默认返回软触发
}

int dvp2_camera::set_trigger_source(QString trigger_source)
{
    if(!m_supported_trigger_source.contains(trigger_source))
    {
        return STATUS_ERROR_PARAMETER; // 如果不匹配任何触发源，返回参数错误 
    }
	dvpTriggerSource value = static_cast<dvpTriggerSource>(m_supported_trigger_source[trigger_source]);
    dvpStatus ret = dvpSetTriggerSource(m_device_handle, value);
	return map_ret_status(ret);
}

int dvp2_camera::start_grab()
{
    dvpStatus ret = dvpStart(m_device_handle);
    if(ret != DVP_STATUS_OK)
    {
        return map_ret_status(ret);
	}
    m_is_grab_running = true;
	return STATUS_SUCCESS;
}

int dvp2_camera::stop_grab()
{
    dvpStatus ret = dvpStop(m_device_handle);
    if (ret != DVP_STATUS_OK)
    {
        return map_ret_status(ret);
    }
    m_is_grab_running = false;
    return STATUS_SUCCESS;
}

bool dvp2_camera::is_grab_running()
{
    return m_is_grab_running;
}

QImage dvp2_camera::get_image(int millisecond)
{
    dvpFrame frame;
    void* p(nullptr);       //帧数据首地址，不需要手动释放内存
    dvpStatus ret = dvpGetFrame(m_device_handle, &frame, &p, millisecond);
    if (ret != DVP_STATUS_OK)
    {
        return QImage();
    }
    if(1)   //测试保存到图像
    {
        dvpSavePicture(&frame, p, "D:/Temp/test.png", 100);
    }

    //将数据保存到 QImage
    QImage img;
    int stride(0);
    switch (frame.format)
    {
    case FORMAT_MONO:
    {
        stride = frame.iWidth; // 灰度图像的 stride 等于宽度
        img = QImage(static_cast<const uchar*>(p), frame.iWidth, frame.iHeight, stride, QImage::Format_Grayscale8).convertToFormat(QImage::Format_RGB888);
        if(0)   //将数据保存到图像
        {
            QString path = QString("D:/Temp/%1.png").arg(frame.uTimestamp);
            img.save(path);  // 保存为 PNG 文件
        }
        break;
    }
    case FORMAT_BGR24:
    {
        stride = frame.iWidth * 3; // 三通道图像的 stride 等于 宽度*3
        img = QImage(static_cast<uchar*>(p), frame.iWidth, frame.iHeight, stride, QImage::Format_BGR888).convertToFormat(QImage::Format_RGB888);
        if (0)   //将数据保存到图像
        {
            QString path = QString("D:/Temp/%1.png").arg(frame.uTimestamp);
            img.save(path);  // 保存为 PNG 文件
        }
        break;
    }
    default:
        // 不支持的格式
        qWarning("Unsupported image format: %d", frame.format);
        break;
    }

    return img;
}

QImage dvp2_camera::trigger_once()
{
    QImage img;
    dvpStatus ret = dvpTriggerFire(m_device_handle);
    if(ret != DVP_STATUS_OK)
    {
        return img;
	}
    img = get_image(3000);
    return img;
}

int dvp2_camera::set_ip_address(unsigned int ip, unsigned int subnet_mask, unsigned int default_gateway)
{
    return 0;
}

int dvp2_camera::set_ip_config(unsigned int type)
{
    return 0;
}
