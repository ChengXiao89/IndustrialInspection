/***********************************
 * 算法参数，加载指定 XML 文件，然后在界面上显示以及控制
 * 算法参数为 key--label--value 的组合
 * 其中 key 为参数关键字，value 为参数值，所调用的算法通过 key--value 进行参数的获取和设置
 *		label 为参数在界面上显示的名称
 *********************************/

#pragma once
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>

#include<QMap>

struct st_parameter
{
	QString m_key{ "" };
	QString m_label{ "" };
	QString m_value{ "" };
};

class algorithm_parameter_widget : public QWidget
{
	Q_OBJECT
public:
	algorithm_parameter_widget(QWidget* parent = nullptr);
	virtual ~algorithm_parameter_widget() override;

	void initialize();		//初始化，首先加载XML文件，然后在界面上显示参数


private:
	// 加载的参数信息，根据XML中的 key--label--value 组合建立两组映射
	// 加载 XML 时，遍历 label 属性在界面上显示，然后设置的时候根据 label 寻找 key 和 value
	// 算法参数更新需要通过 key--value 进行
	QMap<QString, QString> m_map_label_keys;
	QMap<QString, QString> m_map_key_values;
};
