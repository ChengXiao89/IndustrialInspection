#include "new_connection_dlg.h"

#include <QMessageBox>
#include <QRegularExpression>

new_connection_dlg::new_connection_dlg(const QString& server_ip, const quint16& server_port,QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.lineEdit_ip_address->setText(server_ip);
	ui.lineEdit_port->setText(QString("%1").arg(server_port));

	connect(ui.pushButton_ok, &QPushButton::clicked, this, &new_connection_dlg::on_ok);
	connect(ui.pushButton_cancel, &QPushButton::clicked, this, &new_connection_dlg::on_cancel);
}

new_connection_dlg::~new_connection_dlg()
{}

void new_connection_dlg::on_ok()
{
	if(!is_valid_ip_address(ui.lineEdit_ip_address->text()))
	{
		QMessageBox::warning(this, QString::fromStdString("错误"),
			QString::fromStdString("请输入有效的 ip 地址！"), QMessageBox::Ok);
		return;
	}
	m_server_ip = ui.lineEdit_ip_address->text();
	m_server_port = ui.lineEdit_port->text().toUShort();
	m_ret = 0;  //设置返回值为确定
	QDialog::close();
}

void new_connection_dlg::on_cancel()
{
	m_ret = 1;  //设置返回值为取消
	QDialog::close();
}

bool new_connection_dlg::is_valid_ip_address(const QString& ip)
{
	// 用正则检查整体格式：1-3位数字.重复3次+最后1段数字
	static QRegularExpression ipRegex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
	QRegularExpressionMatch match = ipRegex.match(ip);
	if (!match.hasMatch())
		return false;

	// 逐段检查范围 [0, 255]
	for (int i = 1; i <= 4; ++i)
	{
		int value = match.captured(i).toInt();
		if (value < 0 || value > 255)
			return false;
		// 还可加一条禁止前导零（可选）
		if (match.captured(i).startsWith('0') && match.captured(i).length() > 1)
			return false;
	}
	return true;
}