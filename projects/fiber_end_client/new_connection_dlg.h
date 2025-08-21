#pragma once

#include <QDialog>
#include "ui_new_connection_dlg.h"

class new_connection_dlg : public QDialog
{
	Q_OBJECT

public:
	new_connection_dlg(const QString& server_ip, const quint16& server_port, QWidget *parent = nullptr);
	~new_connection_dlg();

	QString m_server_ip;		//服务器 IP 地址
	quint16 m_server_port;		//服务器端口号
	int m_ret{ 1 };  //关闭时的返回值，0 表示确定，1 表示取消

	static bool is_valid_ip_address(const QString& ip);
private slots:
	void on_ok();
	void on_cancel();
private:
	Ui::new_connection ui;
};

