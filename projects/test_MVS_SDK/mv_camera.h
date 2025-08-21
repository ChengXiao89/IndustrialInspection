/************************************************************************/
/* 以C++接口为基础，对常用函数进行二次封装，方便用户使用                */
/************************************************************************/
#pragma once

#include "MvCameraControl.h"
#include <string>

using namespace std;

class mv_camera
{
public:
    mv_camera(MV_CC_DEVICE_INFO* device_info);
    ~mv_camera();

    //判断设备是否可达
    bool is_device_accessible(unsigned int nAccessMode) const;

    //打开设备
    int open();

    //关闭设备
    int close();

    //判断相机是否处于连接状态
    bool is_device_connected() const;

    //注册图像数据回调
    int register_image_callback(void(__stdcall* cbOutput)(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser), void* pUser) const;

    //开启抓图
    int start_grabbing() const;

    //停止抓图
    int stop_grabbing() const;

    //主动获取一帧图像数据
    int get_image_buffer(MV_FRAME_OUT* lp_frame, int millisecond) const;

    //释放图像缓存
    int free_image_buffer(MV_FRAME_OUT* lp_frame) const;

    //显示一帧图像
    int display_one_frame(void* display_window, MV_CC_IMAGE* lp_image_info) const;

    //设置SDK内部图像缓存节点个数
    int set_image_node_num(unsigned int num) const;

    //获取设备信息,取流之前调用
    int get_device_info(MV_CC_DEVICE_INFO* device_info) const;

    //获取相机在网络层面的匹配信息，如数据包丢失率、网络延迟、丢帧情况等.
    //仅支持 GigE 相机和 USB 相机
    int get_all_match_info(void* info) const;

    //获取 GEV（GigE Vision）相机在网络层面的匹配信息，如数据包丢失率、网络延迟、丢帧情况等
    int get_gige_all_match_info(MV_MATCH_INFO_NET_DETECT* match_info_net_detect) const;

    //获取 U3V (USB3 Vision)相机在网络层面的匹配信息，如数据包丢失率、网络延迟、丢帧情况等
    int get_usb_all_match_info(MV_MATCH_INFO_USB_DETECT* match_info_usb_detect) const;

    //获取和设置 int 型参数，如 Width 和 Height
    int get_int_value(IN const char* strKey, OUT MVCC_INTVALUE_EX *value) const;
    int set_int_value(IN const char* strKey, IN int64_t value) const;

    //获取和设置Enum型参数，如 PixelFormat
    int get_enum_value(IN const char* strKey, OUT MVCC_ENUMVALUE *value) const;
    int set_enum_value(IN const char* strKey, IN unsigned int value) const;
    int set_enum_value_by_string(IN const char* strKey, IN const char* value) const;
    int get_enum_entry_symbolic(IN const char* strKey, IN MVCC_ENUMENTRY* pstEnumEntry) const;

    //获取和设置Float型参数，如 ExposureTime和Gain
    int get_float_value(IN const char* strKey, OUT MVCC_FLOATVALUE *value) const;
    int set_float_value(IN const char* strKey, IN float value) const;

    //获取和设置Bool型参数，如 ReverseX
    int get_boolean_value(IN const char* strKey, OUT bool *value) const;
    int set_boolean_value(IN const char* strKey, IN bool value) const;

    //获取和设置String型参数，如 DeviceUserID
    int get_string_value(IN const char* strKey, MVCC_STRINGVALUE *value) const;
    int set_string_value(IN const char* strKey, IN const char *value) const;

    /****************以上是获取基础数据功能，这里封装一些常用的参数获取功能，供外部调用*****************/
    int get_trigger_mode() const;               //获取触发模式
    float get_exposure_time() const;            //获取曝光模式
    float get_gain() const;                     //获取曝光增益
    float get_frame_rate() const;               //
    int get_trigger_source() const;     //
    string get_pixel_format() const;       //获取像素格式


    //执行一次Command型命令，如 UserSetSave
    int execute_command(IN const char* command) const;

    //探测网络最佳包大小(只对GigE相机有效)
    int get_optimal_packet_size(unsigned int* optimal_packet_size) const;

    //注册消息异常回调
    int register_exception_callback(void(__stdcall* exception)(unsigned int msg_type, void* user), void* user) const;

    //注册单个事件回调
    int register_event_callback(const char* event_name, void(__stdcall* event)(MV_EVENT_OUT_INFO * event_info, void* user), void* user) const;

    //强制IP
    int force_ip(unsigned int ip, unsigned int subnet_mask, unsigned int default_gateway) const;

    //配置IP方式
    int set_ip_config(unsigned int type) const;

    //设置网络传输模式
    int set_net_transfer_mode(unsigned int mode) const;

    //像素格式转换
    int convert_pixel_format(MV_CC_PIXEL_CONVERT_PARAM_EX* convert_param) const;

    //保存图片
    int save_image(MV_SAVE_IMAGE_PARAM_EX3* param) const;

    //保存图片到文件
	int save_image_to_file(MV_CC_IMAGE* image, MV_CC_SAVE_IMAGE_PARAM* param, const char* file_path) const;

    //绘制圆形辅助线
    int draw_circle(MVCC_CIRCLE_INFO* circle_info) const;

    //绘制线形辅助线
    int draw_lines(MVCC_LINES_INFO* lines_info) const;

private:

	void* m_device_handle{ nullptr };               //设备句柄，通过设备句柄操作设备，例如打开或者关闭设备
    MV_CC_DEVICE_INFO* m_device_info{ nullptr };    //设备指针，外部传入。本类只负责相关操作，不负责资源释放


};
