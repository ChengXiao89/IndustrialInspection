/***********************************************
 * 加载端面检测配置文件，以设置相关参数.
 * 目前包括 Y轴位置列表，每张影像端面数量， 影像保存路径
 ***********************************************/

#pragma once
#include <vector>
#include <string>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

 // 端面检测配置参数
struct st_config_data
{
	int m_light_brightness{ 8000 };						//光源亮度
	int m_move_speed{ 1000 };							//移动速度
	int m_move_step_x{ 1000 }, m_move_step_y{ 1000 };	//上下调整时的移动步长
	std::vector<int> m_position_list;					//Y 轴上的位置列表，以Y轴端点作为起点，运行状态下，会依次在此位置对焦-检测
	int m_fiber_end_count{ 4 };							//每张影像上的端面数量. 自动对焦以及后续检测需要使用的参数
	int m_auto_detect{ 1 };								//对焦完成之后是否自动执行检测 0 -- 否    1 -- 是
	std::string m_save_path{ "./saveimages" };		//指定保存拍照图像的路径

	std::string m_config_file_path{ "./config.xml" };		//配置文件路径,服务刚启动之后会加载配置文件，只在调用 load_from_file 时初始化一次

	//服务刚启动的时候加载配置文件，保存文件路径以及数据
	bool load_from_file(const std::string& config_file_path)
	{
		QFile file(QString::fromStdString(config_file_path));
		if (!file.open(QIODevice::ReadOnly)) 
		{
			qDebug() << "Failed to open config file";
			return false;
		}
		QDomDocument doc;
		if (!doc.setContent(&file)) 
		{
			file.close();
			qDebug() << "Failed to parse XML content";
			return false;
		}
		file.close();
		m_config_file_path = config_file_path;

		QDomElement root = doc.documentElement();
		// 光源亮度
		QDomNode light_brightness_node = root.namedItem("light_brightness");
		if (!light_brightness_node.isNull())
		{
			m_light_brightness = light_brightness_node.toElement().text().toInt();
		}
		// 运动速度
		QDomNode move_speed_node = root.namedItem("move_speed");
		if (!move_speed_node.isNull())
		{
			m_move_speed = move_speed_node.toElement().text().toInt();
		}
		// 移动步长-X
		QDomNode move_step_x_node = root.namedItem("move_step_x");
		if (!move_step_x_node.isNull())
		{
			m_move_step_x = move_step_x_node.toElement().text().toInt();
		}
		// 移动步长-Y
		QDomNode move_step_y_node = root.namedItem("move_step_y");
		if (!move_step_y_node.isNull())
		{
			m_move_step_y = move_step_y_node.toElement().text().toInt();
		}
		// 位置列表
		QDomNodeList positions_node = root.elementsByTagName("position");
		for (int i = 0; i < positions_node.count(); ++i) 
		{
			QDomElement elem = positions_node.at(i).toElement();
			m_position_list.push_back(elem.text().toInt());
		}
		// 端面数量
		QDomNode count_node = root.namedItem("fiber_end_count");
		if (!count_node.isNull()) 
		{
			m_fiber_end_count = count_node.toElement().text().toInt();
		}
		// 自动检测
		QDomNode auto_detect_node = root.namedItem("auto_detect");
		if (!auto_detect_node.isNull())
		{
			m_auto_detect = auto_detect_node.toElement().text().toInt();
		}
		// 保存路径
		QDomNode save_path_node = root.namedItem("save_path");
		if (!save_path_node.isNull()) 
		{
			m_save_path = save_path_node.toElement().text().toStdString();
		}
		return true;
	}

	//界面上修改相关配置之后更新数据，然后保存到文件
	bool save() const
	{
		QFile file(QString::fromStdString(m_config_file_path));
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) 
		{
			qDebug() << "Failed to open config file for writing";
			return false;
		}
		QDomDocument doc;
		// 创建根节点
		QDomElement root = doc.createElement("config");
		doc.appendChild(root);
		// 保存 光源亮度
		QDomElement light_brightness_node = doc.createElement("light_brightness");
		light_brightness_node.appendChild(doc.createTextNode(QString::number(m_light_brightness)));
		root.appendChild(light_brightness_node);
		// 保存 运动速度
		QDomElement move_speed_node = doc.createElement("move_speed");
		move_speed_node.appendChild(doc.createTextNode(QString::number(m_move_speed)));
		root.appendChild(move_speed_node);
		// 保存 移动步长-X
		QDomElement move_step_x_node = doc.createElement("move_step_x");
		move_step_x_node.appendChild(doc.createTextNode(QString::number(m_move_step_x)));
		root.appendChild(move_step_x_node);
		// 保存 移动步长-Y
		QDomElement move_step_y_node = doc.createElement("move_step_y");
		move_step_y_node.appendChild(doc.createTextNode(QString::number(m_move_step_y)));
		root.appendChild(move_step_y_node);
		// 保存 位置列表
		for (int pos : m_position_list) 
		{
			QDomElement position_node = doc.createElement("position");
			position_node.appendChild(doc.createTextNode(QString::number(pos)));
			root.appendChild(position_node);
		}
		// 保存 端面数量
		QDomElement count_node = doc.createElement("fiber_end_count");
		count_node.appendChild(doc.createTextNode(QString::number(m_fiber_end_count)));
		root.appendChild(count_node);

		// 保存 自动检测
		QDomElement auto_detect_node = doc.createElement("auto_detect");
		auto_detect_node.appendChild(doc.createTextNode(QString::number(m_auto_detect)));
		root.appendChild(auto_detect_node);

		// 添加 保存路径
		QDomElement save_path_node = doc.createElement("save_path");
		save_path_node.appendChild(doc.createTextNode(QString::fromStdString(m_save_path)));
		root.appendChild(save_path_node);

		// 写入文件
		QTextStream out(&file);
		doc.save(out, 4);  // 保存格式化的 XML,第二个参数表示缩进的空格数，用于控制 QDomDocument::save() 输出 XML 时的缩进格式
		file.close();

		return true;
	}

	QJsonObject save_to_json() const
	{
		QJsonObject root;
		
		root["light_brightness"] = m_light_brightness;
		root["move_speed"] = m_move_speed;
		root["move_step_x"] = m_move_step_x;
		root["move_step_y"] = m_move_step_y;
		QJsonArray positions_array;
		for (int pos : m_position_list)
		{
			positions_array.append(pos);
		}
		root["position_list"] = positions_array;
		root["fiber_end_count"] = m_fiber_end_count;
		root["auto_detect"] = m_auto_detect;
		root["save_path"] = QString::fromStdString(m_save_path);

		return root;
	}
};