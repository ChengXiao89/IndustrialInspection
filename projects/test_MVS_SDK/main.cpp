#include <QtCore/QCoreApplication>
#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
using namespace std;

#include "MvCameraControl.h"
#include "mv_camera.h"

void setup_console_for_utf8();

int initialize_sdk();
int finalize_sdk();

MV_CC_DEVICE_INFO_LIST get_devices_info();			//获取所有连接的相机设备信息
void open_device(MV_CC_DEVICE_INFO* device_info);	//打开相机

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
	//setup_console_for_utf8();
    if (MV_OK != initialize_sdk())
    {
        cout << "initialize_sdk fail!" << endl;
		return -1;
    }
	//枚举设备
	MV_CC_DEVICE_INFO_LIST device_list = get_devices_info();
	//打开第一个设备
	if(device_list.nDeviceNum > 0)
	{
		open_device(device_list.pDeviceInfo[0]);
	}

    
    if (MV_OK != finalize_sdk())
    {
        cout << "finalize_sdk fail!" << endl;
        return -1;
    }
    return 0;
}

int initialize_sdk()
{
	return MV_CC_Initialize();
}

int finalize_sdk()
{
	return MV_CC_Finalize();
}

MV_CC_DEVICE_INFO_LIST get_devices_info()
{
	/**************************** 1.枚举设备 **************************/
	MV_CC_DEVICE_INFO_LIST stDeviceList{ 0 };
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	//这里需要移除bin目录下dll(可能是因为需要依赖其他dll但没有拷贝进来)，否则会返回错误值
	int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);	
	if(nRet != MV_OK)
	{
		cout << "EnumDevices fail! nRet = " << nRet << endl;
		return stDeviceList;
	}
	/************************ 2.打印所有设备信息 **********************/
	for (int i = 0; i < stDeviceList.nDeviceNum; i++)
	{
		MV_CC_DEVICE_INFO* device_info = stDeviceList.pDeviceInfo[i];
		if (device_info == nullptr)
		{
			continue;
		}
		//GIGE设备，设备类型记录在 nTLayerType 字段
		if (device_info->nTLayerType == MV_GIGE_DEVICE)
		{
			//获取 ip 地址
			int nIp1 = ((static_cast<unsigned int>(device_info->SpecialInfo.stGigEInfo.nCurrentIp) & 0xff000000) >> 24);
			int nIp2 = ((static_cast<unsigned int>(device_info->SpecialInfo.stGigEInfo.nCurrentIp) & 0x00ff0000) >> 16);
			int nIp3 = ((static_cast<unsigned int>(device_info->SpecialInfo.stGigEInfo.nCurrentIp) & 0x0000ff00) >> 8);
			int nIp4 = (static_cast<unsigned int>(device_info->SpecialInfo.stGigEInfo.nCurrentIp) & 0x000000ff);
			std::string str_ip =	std::to_string(nIp1) + "." + 
									std::to_string(nIp2) + "." + 
									std::to_string(nIp3) + "." + 
									std::to_string(nIp4);
			//获取用户名，sn码和型号
			std::string user_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stGigEInfo.chUserDefinedName), 
				sizeof(device_info->SpecialInfo.stGigEInfo.chUserDefinedName));
			std::string serial_number(reinterpret_cast<const char*>(device_info->SpecialInfo.stGigEInfo.chSerialNumber), 
				sizeof(device_info->SpecialInfo.stGigEInfo.chSerialNumber));
			std::string model_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stGigEInfo.chModelName), 
				sizeof(device_info->SpecialInfo.stGigEInfo.chModelName));
			cout << "device info:" << endl;
			cout << "ip: " << str_ip << endl;
			cout << "user_name: " << user_name << endl;
			cout << "model_name: " << model_name << endl;
			cout << "serial_number: " << serial_number << endl;
		}
		//USB设备，USB设备没有 ip
		else if (device_info->nTLayerType == MV_USB_DEVICE)
		{
			//获取用户名，sn码和型号
			std::string user_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stUsb3VInfo.chUserDefinedName),
				sizeof(device_info->SpecialInfo.stUsb3VInfo.chUserDefinedName));
			std::string serial_number(reinterpret_cast<const char*>(device_info->SpecialInfo.stUsb3VInfo.chSerialNumber),
				sizeof(device_info->SpecialInfo.stUsb3VInfo.chSerialNumber));
			std::string model_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stUsb3VInfo.chModelName),
				sizeof(device_info->SpecialInfo.stUsb3VInfo.chModelName));
			cout << "device info:" << endl;
			cout << "user_name: " << user_name << endl;
			cout << "model_name: " << model_name << endl;
			cout << "serial_number: " << serial_number << endl;
		}
		//基于 GenTL 标准、采用 Camera Link 接口的工业相机类型
		else if (device_info->nTLayerType == MV_GENTL_CAMERALINK_DEVICE)
		{
			//获取用户名，sn码和型号
			std::string user_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stCMLInfo.chUserDefinedName),
				sizeof(device_info->SpecialInfo.stCMLInfo.chUserDefinedName));
			std::string serial_number(reinterpret_cast<const char*>(device_info->SpecialInfo.stCMLInfo.chSerialNumber),
				sizeof(device_info->SpecialInfo.stCMLInfo.chSerialNumber));
			std::string model_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stCMLInfo.chModelName),
				sizeof(device_info->SpecialInfo.stCMLInfo.chModelName));
			cout << "device info:" << endl;
			cout << "user_name: " << user_name << endl;
			cout << "model_name: " << model_name << endl;
			cout << "serial_number: " << serial_number << endl;
		}
		//基于 GenTL 标准、采用 CoaXPress（CXP）接口的工业相机类型
		else if (device_info->nTLayerType == MV_GENTL_CXP_DEVICE)
		{
			//获取用户名，sn码和型号
			std::string user_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stCXPInfo.chUserDefinedName),
				sizeof(device_info->SpecialInfo.stCXPInfo.chUserDefinedName));
			std::string serial_number(reinterpret_cast<const char*>(device_info->SpecialInfo.stCXPInfo.chSerialNumber),
				sizeof(device_info->SpecialInfo.stCXPInfo.chSerialNumber));
			std::string model_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stCXPInfo.chModelName),
				sizeof(device_info->SpecialInfo.stCXPInfo.chModelName));
			cout << "device info:" << endl;
			cout << "user_name: " << user_name << endl;
			cout << "model_name: " << model_name << endl;
			cout << "serial_number: " << serial_number << endl;
		}
		//基于 GenTL 标准、采用海康威视自研 XoF 传输协议的工业相机
		else if (device_info->nTLayerType == MV_GENTL_XOF_DEVICE)
		{
			//获取用户名，sn码和型号
			std::string user_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stXoFInfo.chUserDefinedName),
				sizeof(device_info->SpecialInfo.stXoFInfo.chUserDefinedName));
			std::string serial_number(reinterpret_cast<const char*>(device_info->SpecialInfo.stXoFInfo.chSerialNumber),
				sizeof(device_info->SpecialInfo.stXoFInfo.chSerialNumber));
			std::string model_name(reinterpret_cast<const char*>(device_info->SpecialInfo.stXoFInfo.chModelName),
				sizeof(device_info->SpecialInfo.stXoFInfo.chModelName));
			cout << "device info:" << endl;
			cout << "user_name: " << user_name << endl;
			cout << "model_name: " << model_name << endl;
			cout << "serial_number: " << serial_number << endl;
		}
		else
		{
			cout << "index " << i << " : Unknown device enumerated" << endl;
		}
	}
	return stDeviceList;
}

void open_device(MV_CC_DEVICE_INFO* device_info)
{
	if (device_info == nullptr)
	{
		cout << "device is invalid!" << endl;
		return;
	}
	mv_camera cam(device_info);
	int nRet = cam.open();
	if (MV_OK != nRet)
	{
		cout << "open device fail!" << endl;
		return;
	}
	// ch:探测网络最佳包大小(只对GigE相机有效)
	unsigned int nPacketSize = 0;
	nRet = cam.get_optimal_packet_size(&nPacketSize);
	if (nRet != MV_OK)
	{
		cout << "get_optimal_packet_size error: " << nRet << endl;
	}
	else
	{
		cout << "网络最佳包大小: " << nPacketSize << endl;
	}
	/***********************************获取基础参数信息***********************************/
	int trigger_mode = cam.get_trigger_mode();
	float exposure_time = cam.get_exposure_time();
	float gain = cam.get_gain();
	float frame_rate = cam.get_frame_rate();
	int trigger_source = cam.get_trigger_source();
	string pixel_format = cam.get_pixel_format();
	cout << "相机参数......" << endl;
	cout << "触发模式: " << trigger_mode << endl;
	cout << "曝光时间: " << exposure_time << endl;
	cout << "曝光增益: " << gain << endl;
	cout << "帧率: " << frame_rate << endl;
	cout << "触发源: " << trigger_source << endl;
	cout << "像素格式: " << pixel_format << endl;
}

void setup_console_for_utf8()
{
#if defined(_WIN32) || defined(_WIN64)
	// 设置输出编码为 UTF-8
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	// 使 std::cout 使用 UTF-8 编码
	std::ios_base::sync_with_stdio(false);
	std::wcin.imbue(std::locale(".UTF8"));
	std::wcout.imbue(std::locale(".UTF8"));
#endif
	// Linux/macOS 一般默认是 UTF-8，无需设置
}