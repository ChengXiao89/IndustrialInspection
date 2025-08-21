#pragma once

#include <QDialog>
#include "ui_ip_config_dialog.h"

#include <string>
using namespace std;

class ip_config_dialog : public QDialog
{
	Q_OBJECT

public:
	ip_config_dialog(const QString& str_ip, const QString& str_mask,const QString& str_gate,QWidget *parent = nullptr);
	~ip_config_dialog();

	void initialize() const;		//初始化界面

	//设置静态 ip 时编辑框中的值, 整数类型，用于传递给相机SDK设置ip
	unsigned m_ip_address{ 0 };
	unsigned m_subnet_mask{ 0 };
	unsigned m_default_gateway{ 0 };
	//设置静态 ip 时编辑框中的值, 字符串类型，用于更新相机信息
	QString m_str_ip_address{ "" };
	QString m_str_subnet_mask{ "" };
	QString m_str_default_gateway{ "" };

	/*******************************
	 * 在设置为静态 ip 情况下点击确定时需要检查设置参数的有效性
	 * 1. 所有地址必须是 XXX.XXX.XXX.XXX 格式，且必须是整数字，取值范围[0,255]
	 * 2. 子网掩码的二进制必须是连续的1，再连续的0 ,例如 255.255.255.0/255.255.0.0
	 * 3. ip 地址和网关需要在同一网段才能通信 ip & netmask == gateway & netmask
	 * 返回值： 1--地址无效 2--ip与网关冲突或无法通信 0--正常
	 ********************************/
	int check_static_ip_valid();
	static bool is_valid_ip_address(const QString& ip);				//判断 ip 地址是否合法
	static bool is_subnet_mask_valid(const QString& subnet_mask);	//判断子网掩码是否合法
	static bool is_same_subnet(const QString& ip, const QString& gateway, const QString& netmask);		//判断 ip 地址与网关能否通信
	static qint64 ip_string_to_int(const QString& ip);				//将 ip 字符串转换成整数

	int config_type{ -1 };		//设置方式	0:静态ip	1:DHCP	2:LLA  -1:取消设置
	

private slots:
	void on_set_static_ip() const;
	void on_set_dhcp() const;
	void on_set_lla() const;
	void on_ok();
	void on_cancel();

private:
	Ui::ip_config_dialogClass ui;
};

