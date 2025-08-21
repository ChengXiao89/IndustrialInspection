#include "ip_config_dialog.h"

#include <QMessageBox>
#include <QRegularExpression>


ip_config_dialog::ip_config_dialog(const QString& str_ip, const QString& str_mask, const QString& str_gateway, QWidget *parent)
	: QDialog(parent), m_str_ip_address(str_ip),m_str_subnet_mask(str_mask),m_str_default_gateway(str_gateway)
{
	ui.setupUi(this);
	initialize();

	//注册消息
	connect(ui.radioButton_static, &QRadioButton::pressed, this, &ip_config_dialog::on_set_static_ip);
	connect(ui.radioButton_dhcp, &QRadioButton::pressed, this, &ip_config_dialog::on_set_dhcp);
	connect(ui.radioButton_lla, &QRadioButton::pressed, this, &ip_config_dialog::on_set_lla);
	connect(ui.pushButton_OK, &QPushButton::clicked, this, &ip_config_dialog::on_ok);
	connect(ui.pushButton_Cancel, &QPushButton::clicked, this, &ip_config_dialog::on_cancel);
}

ip_config_dialog::~ip_config_dialog()
{}

void ip_config_dialog::initialize() const
{
	ui.radioButton_static->setChecked(true);
	ui.lineEdit_ip_address->setText(m_str_ip_address);
	ui.lineEdit_subnet_mask->setText(m_str_subnet_mask);
	ui.lineEdit_default_gate->setText(m_str_default_gateway);
}

void ip_config_dialog::on_set_static_ip() const
{
	ui.radioButton_static->setChecked(true);
	ui.radioButton_dhcp->setChecked(false);
	ui.radioButton_lla->setChecked(false);
	ui.groupbox_ip_config->setEnabled(true);
}

void ip_config_dialog::on_set_dhcp() const
{
	ui.radioButton_static->setChecked(false);
	ui.radioButton_dhcp->setChecked(true);
	ui.radioButton_lla->setChecked(false);
	ui.groupbox_ip_config->setEnabled(false);
}

void ip_config_dialog::on_set_lla() const
{
	ui.radioButton_static->setChecked(false);
	ui.radioButton_dhcp->setChecked(false);
	ui.radioButton_lla->setChecked(true);
	ui.groupbox_ip_config->setEnabled(false);
}

void ip_config_dialog::on_ok()
{
	if (ui.radioButton_static->isChecked())
	{
		if(check_static_ip_valid() != 0)
		{
			return;
		}
		config_type = 0;
		m_ip_address = static_cast<unsigned>(ip_string_to_int(ui.lineEdit_ip_address->text()));
		m_default_gateway = static_cast<unsigned>(ip_string_to_int(ui.lineEdit_default_gate->text()));
		m_subnet_mask = static_cast<unsigned>(ip_string_to_int(ui.lineEdit_subnet_mask->text()));

		m_str_ip_address = ui.lineEdit_ip_address->text();
		m_str_default_gateway = ui.lineEdit_default_gate->text();
		m_str_subnet_mask = ui.lineEdit_subnet_mask->text();
	}
	else if (ui.radioButton_dhcp->isChecked())
	{
		config_type = 1;
	}
	if (ui.radioButton_lla->isChecked())
	{
		config_type = 2;
	}

	QDialog::close();
}

void ip_config_dialog::on_cancel()
{
	config_type = -1;
	QDialog::close();
}

int ip_config_dialog::check_static_ip_valid()
{
	/*************************1.判断ip地址、子网掩码、默认网关是否合法****************************/
	if(!is_valid_ip_address(ui.lineEdit_ip_address->text()))
	{
		QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("ip 地址无效!"), QMessageBox::Ok);
		return 1;
	}
	if (!is_subnet_mask_valid(ui.lineEdit_subnet_mask->text()))
	{
		QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("子网掩码无效!"), QMessageBox::Ok);
		return 1;
	}
	if (!is_valid_ip_address(ui.lineEdit_default_gate->text()))
	{
		QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("默认网关无效!"), QMessageBox::Ok);
		return 1;
	}
	/*************************2.判断ip地址与默认网关是否冲突以及能否通信****************************/
	if (ui.lineEdit_ip_address->text() == ui.lineEdit_default_gate->text())
	{
		QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("ip 地址与默认网关冲突!"), QMessageBox::Ok);
		return 2;
	}
	if(!is_same_subnet(ui.lineEdit_ip_address->text(), ui.lineEdit_default_gate->text(), ui.lineEdit_subnet_mask->text()))
	{
		QMessageBox::information(this, QString::fromStdString("错误"), QString::fromStdString("ip 地址与默认网关无法通信!"), QMessageBox::Ok);
		return 2;
	}
	return 0;
}

bool ip_config_dialog::is_valid_ip_address(const QString& ip)
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

bool ip_config_dialog::is_subnet_mask_valid(const QString& subnet_mask)
{
	if(!is_valid_ip_address(subnet_mask))
	{
		return false;
	}
	// 转换为 32 位二进制字符串
	QStringList parts = subnet_mask.split('.');
	QString binary_str;
	for (const QString& part : parts) 
	{
		int num = part.toInt();
		if (num < 0 || num > 255) return false;
		binary_str += QString("%1").arg(num, 8, 2, QChar('0'));
	}
	// 查找第一处 0 出现的位置
	qsizetype pos = binary_str.indexOf('0');
	if (pos == -1) 
		return true; // 全是 1，比如 255.255.255.255
	// 如果在 0 之后还有 1，则非法
	return binary_str.indexOf('1', pos) == -1;
}

bool ip_config_dialog::is_same_subnet(const QString& ip, const QString& gateway, const QString& netmask)
{
	qint64  int_ip = ip_string_to_int(ip);
	qint64 int_gateway = ip_string_to_int(gateway);
	qint64 int_mask = ip_string_to_int(netmask);
	if(int_ip == -1 || int_gateway == -1 || int_mask == -1)
	{
		return false;
	}
	return (int_ip & int_mask) == (int_gateway & int_mask);
}

qint64 ip_config_dialog::ip_string_to_int(const QString& ip)
{
	QStringList parts = ip.split('.');
	if (parts.size() != 4)
		return -1;
	long long result = 0;
	for (int i = 0; i < 4; ++i) 
	{
		int part = parts[i].toInt();
		if (part < 0 || part > 255)
			return -1;
		result |= (part << ((3 - i) * 8));
	}
	return result;
}