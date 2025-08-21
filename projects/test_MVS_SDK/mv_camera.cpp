#include "mv_camera.h"

mv_camera::mv_camera(MV_CC_DEVICE_INFO* device_info):m_device_info(device_info)
{
    
}

mv_camera::~mv_camera()
{
    if (m_device_handle)
    {
        MV_CC_DestroyHandle(m_device_handle);
        m_device_handle = nullptr;
    }
    m_device_info = nullptr;
}

bool mv_camera::is_device_accessible(unsigned int nAccessMode) const
{
	if (m_device_info == nullptr)
	{
		return false;
	}
    return MV_CC_IsDeviceAccessible(m_device_info, nAccessMode);
}

int mv_camera::open()
{
    if (m_device_info == nullptr)
    {
        return MV_E_PARAMETER;
    }
    if (m_device_handle != nullptr)
    {
        return MV_E_CALLORDER;
    }
    //创建设备句柄
    int nRet  = MV_CC_CreateHandle(&m_device_handle, m_device_info);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    //根据设备句柄打开设备
    nRet = MV_CC_OpenDevice(m_device_handle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_device_handle);
        m_device_handle = nullptr;
    }
    return nRet;
}

int mv_camera::close()
{
    if (nullptr == m_device_handle)
    {
        return MV_E_HANDLE;
    }
    MV_CC_CloseDevice(m_device_handle);
    int nRet = MV_CC_DestroyHandle(m_device_handle);
    m_device_handle = nullptr;
    return nRet;
}

bool mv_camera::is_device_connected() const
{
    return MV_CC_IsDeviceConnected(m_device_handle);
}

int mv_camera::register_image_callback(void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser), void* pUser) const
{
    return MV_CC_RegisterImageCallBackEx(m_device_handle, cbOutput, pUser);
}

int mv_camera::start_grabbing() const
{
    return MV_CC_StartGrabbing(m_device_handle);
}

int mv_camera::stop_grabbing() const
{
    return MV_CC_StopGrabbing(m_device_handle);
}

int mv_camera::get_image_buffer(MV_FRAME_OUT* lp_frame, int millisecond) const
{
    return MV_CC_GetImageBuffer(m_device_handle, lp_frame, millisecond);
}

int mv_camera::free_image_buffer(MV_FRAME_OUT* lp_frame) const
{
    return MV_CC_FreeImageBuffer(m_device_handle, lp_frame);
}

int mv_camera::display_one_frame(void* display_window,MV_CC_IMAGE* lp_image_info) const
{
	return MV_CC_DisplayOneFrameEx2(m_device_handle, display_window, lp_image_info,0);
   
}

int mv_camera::set_image_node_num(unsigned int num) const
{
    return MV_CC_SetImageNodeNum(m_device_handle, num);
}

int mv_camera::get_device_info(MV_CC_DEVICE_INFO* device_info) const
{
    return MV_CC_GetDeviceInfo(m_device_handle, device_info);
}

int mv_camera::get_all_match_info(void* info) const
{
    if(info == nullptr)
    {
        return MV_E_PARAMETER;
    }
    MV_ALL_MATCH_INFO match_info = { 0,nullptr,0 };
    if (m_device_info->nTLayerType == MV_GIGE_DEVICE)
    {
        match_info.nType = MV_MATCH_TYPE_NET_DETECT;
        match_info.pInfo = info;
        match_info.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);
        memset(match_info.pInfo, 0, sizeof(MV_MATCH_INFO_NET_DETECT));
    }
    else if(m_device_info->nTLayerType == MV_USB_DEVICE)
    {
        match_info.nType = MV_MATCH_TYPE_USB_DETECT;
        match_info.pInfo = info;
        match_info.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);
        memset(match_info.pInfo, 0, sizeof(MV_MATCH_INFO_USB_DETECT));
    }
    return MV_CC_GetAllMatchInfo(m_device_handle, &match_info);
}

int mv_camera::get_gige_all_match_info(MV_MATCH_INFO_NET_DETECT* match_info_net_detect) const
{
    if (match_info_net_detect == nullptr)
    {
        return MV_E_PARAMETER;
    }
    MV_CC_DEVICE_INFO device_info = {0};
    get_device_info(&device_info);
    if (device_info.nTLayerType != MV_GIGE_DEVICE)
    {
        return MV_E_SUPPORT;
    }
    MV_ALL_MATCH_INFO match_info = {0};
    match_info.nType = MV_MATCH_TYPE_NET_DETECT;
    match_info.pInfo = match_info_net_detect;
    match_info.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);
    memset(match_info.pInfo, 0, sizeof(MV_MATCH_INFO_NET_DETECT));

    return MV_CC_GetAllMatchInfo(m_device_handle, &match_info);
}

int mv_camera::get_usb_all_match_info(MV_MATCH_INFO_USB_DETECT* match_info_usb_detect) const
{
	if (match_info_usb_detect == nullptr)
	{
	    return MV_E_PARAMETER;
	}
	MV_CC_DEVICE_INFO device_info = {0};
	get_device_info(&device_info);
	if (device_info.nTLayerType != MV_USB_DEVICE)
	{
	    return MV_E_SUPPORT;
	}
	MV_ALL_MATCH_INFO match_info = {0};
	match_info.nType = MV_MATCH_TYPE_USB_DETECT;
	match_info.pInfo = match_info_usb_detect;
	match_info.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);
	memset(match_info.pInfo, 0, sizeof(MV_MATCH_INFO_USB_DETECT));

	return MV_CC_GetAllMatchInfo(m_device_handle, &match_info);
}

int mv_camera::get_int_value(IN const char* strKey, OUT MVCC_INTVALUE_EX *value) const
{
    return MV_CC_GetIntValueEx(m_device_handle, strKey, value);
}

int mv_camera::set_int_value(IN const char* strKey, IN int64_t value) const
{
    return MV_CC_SetIntValueEx(m_device_handle, strKey, value);
}

int mv_camera::get_enum_value(IN const char* strKey, OUT MVCC_ENUMVALUE * value) const
{
    return MV_CC_GetEnumValue(m_device_handle, strKey, value);
}

int mv_camera::set_enum_value(IN const char* strKey, IN unsigned int value) const
{
    return MV_CC_SetEnumValue(m_device_handle, strKey, value);
}

int mv_camera::set_enum_value_by_string(IN const char* strKey, IN const char* value) const
{
    return MV_CC_SetEnumValueByString(m_device_handle, strKey, value);
}

int mv_camera::get_enum_entry_symbolic(IN const char* strKey, IN MVCC_ENUMENTRY* pstEnumEntry) const
{
    return MV_CC_GetEnumEntrySymbolic(m_device_handle, strKey, pstEnumEntry);
}

int mv_camera::get_float_value(IN const char* strKey, OUT MVCC_FLOATVALUE *value) const
{
    return MV_CC_GetFloatValue(m_device_handle, strKey, value);
}

int mv_camera::set_float_value(IN const char* strKey, IN float value) const
{
    return MV_CC_SetFloatValue(m_device_handle, strKey, value);
}

int mv_camera::get_boolean_value(IN const char* strKey, OUT bool *value) const
{
    return MV_CC_GetBoolValue(m_device_handle, strKey, value);
}

int mv_camera::set_boolean_value(IN const char* strKey, IN bool value) const
{
    return MV_CC_SetBoolValue(m_device_handle, strKey, value);
}

int mv_camera::get_string_value(IN const char* strKey, MVCC_STRINGVALUE *value) const
{
    return MV_CC_GetStringValue(m_device_handle, strKey, value);
}

int mv_camera::set_string_value(IN const char* strKey, IN const char* value) const
{
    return MV_CC_SetStringValue(m_device_handle, strKey, value);
}

/****************************一些基础参数获取******************************/
int mv_camera::get_trigger_mode() const
{
    MVCC_ENUMVALUE value = { 0 };
    const int nRet = get_enum_value("TriggerMode", &value);
    if (MV_OK != nRet)
    {
        return -1;
    }
    return static_cast<int>(value.nCurValue);
}
float mv_camera::get_exposure_time() const
{
    MVCC_FLOATVALUE value = { 0 };

    const int nRet = get_float_value("ExposureTime", &value);
    if (MV_OK != nRet)
    {
        return -1.0;
    }
    return value.fCurValue;
}
float mv_camera::get_gain() const
{
    MVCC_FLOATVALUE value = { 0 };

    const int nRet = get_float_value("Gain", &value);
    if (MV_OK != nRet)
    {
        return -1.0;
    }
    return value.fCurValue;
}
float mv_camera::get_frame_rate() const
{
    MVCC_FLOATVALUE value = { 0 };
    const int nRet = get_float_value("ResultingFrameRate", &value);
    if (MV_OK != nRet)
    {
        return -1.0;
    }
    return value.fCurValue;
}
int mv_camera::get_trigger_source() const
{
    MVCC_ENUMVALUE value = { 0 };

    const int nRet = get_enum_value("TriggerSource", &value);
    if (MV_OK != nRet)
    {
        return -1;
    }
    return static_cast<int>(value.nCurValue);
}
string mv_camera::get_pixel_format() const
{
    MVCC_ENUMVALUE value = { 0 };
    MVCC_ENUMENTRY pixel_format_info = { 0 };

	int nRet = get_enum_value("PixelFormat", &value);
    if (MV_OK != nRet)
    {
        return "error_pixel_format";
    }

    pixel_format_info.nValue = value.nCurValue;
    nRet = get_enum_entry_symbolic("PixelFormat", &pixel_format_info);
    if (MV_OK != nRet)
    {
        return "error_pixel_format";
    }
    return pixel_format_info.chSymbolic;
}

int mv_camera::execute_command(IN const char* command) const
{
    return MV_CC_SetCommandValue(m_device_handle, command);
}

int mv_camera::get_optimal_packet_size(unsigned int* optimal_packet_size) const
{
	if (m_device_info->nTLayerType != MV_GIGE_DEVICE)
	{
        return MV_E_SUPPORT;
	}
    if (optimal_packet_size == nullptr)
    {
        return MV_E_PARAMETER;
    }
    int nRet = MV_CC_GetOptimalPacketSize(m_device_handle);
    if (nRet < MV_OK)
    {
        return nRet;
    }
    *optimal_packet_size = static_cast<unsigned int>(nRet);
    return MV_OK;
}

int mv_camera::register_exception_callback(void(__stdcall* exception)(unsigned int msg_type, void* user), void* user) const
{
    return MV_CC_RegisterExceptionCallBack(m_device_handle, exception, user);
}

int mv_camera::register_event_callback(const char* event_name, void(__stdcall* event)(MV_EVENT_OUT_INFO* event_info, void* user), void* user) const
{
    return MV_CC_RegisterEventCallBackEx(m_device_handle, event_name, event, user);
}

int mv_camera::force_ip(unsigned int ip, unsigned int subnet_mask, unsigned int default_gateway) const
{
    return MV_GIGE_ForceIpEx(m_device_handle, ip, subnet_mask, default_gateway);
}

int mv_camera::set_ip_config(unsigned int type) const
{
    return MV_GIGE_SetIpConfig(m_device_handle, type);
}

int mv_camera::set_net_transfer_mode(unsigned int mode) const
{
    return MV_GIGE_SetNetTransMode(m_device_handle, mode);
}

int mv_camera::convert_pixel_format(MV_CC_PIXEL_CONVERT_PARAM_EX* convert_param) const
{
    return MV_CC_ConvertPixelTypeEx(m_device_handle, convert_param);
}

int mv_camera::save_image(MV_SAVE_IMAGE_PARAM_EX3* param) const
{
    return MV_CC_SaveImageEx3(m_device_handle, param);
}

int mv_camera::save_image_to_file(MV_CC_IMAGE* image, MV_CC_SAVE_IMAGE_PARAM* param, const char* file_path) const
{
	return MV_CC_SaveImageToFileEx2(m_device_handle, image, param, file_path);
}

int mv_camera::draw_circle(MVCC_CIRCLE_INFO* circle_info) const
{
    return MV_CC_DrawCircle(m_device_handle, circle_info);
}

int mv_camera::draw_lines(MVCC_LINES_INFO* lines_info) const
{
    return MV_CC_DrawLines(m_device_handle, lines_info);
}
