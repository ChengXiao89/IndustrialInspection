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
	std::vector<int> m_position_list;					//Y 轴上的位置列表，以Y轴端点作为起点，运行状态下，会依次在此位置对焦-检测
	int m_count{ 4 };									//每张影像上的端面数量. 自动对焦以及后续检测需要使用的参数
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
		// 读取 position_list
		QDomNodeList positions_node = root.elementsByTagName("positions");
		for (int i = 0; i < positions_node.count(); ++i) 
		{
			QDomElement elem = positions_node.at(i).toElement();
			m_position_list.push_back(elem.text().toInt());
		}
		// 读取 m_count
		QDomNode count_node = root.namedItem("count");
		if (!count_node.isNull()) 
		{
			m_count = count_node.toElement().text().toInt();
		}
		// 读取 m_auto_detect
		QDomNode auto_detect_node = root.namedItem("auto_detect");
		if (!auto_detect_node.isNull())
		{
			m_auto_detect = auto_detect_node.toElement().text().toInt();
		}
		// 读取 m_save_path
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
		// 保存 m_position_list
		for (int pos : m_position_list) 
		{
			QDomElement position_node = doc.createElement("position");
			position_node.appendChild(doc.createTextNode(QString::number(pos)));
			root.appendChild(position_node);
		}
		// 保存 m_count
		QDomElement count_node = doc.createElement("count");
		count_node.appendChild(doc.createTextNode(QString::number(m_count)));
		root.appendChild(count_node);

		// 保存 m_auto_detect
		QDomElement auto_detect_node = doc.createElement("auto_detect");
		auto_detect_node.appendChild(doc.createTextNode(QString::number(m_auto_detect)));
		root.appendChild(auto_detect_node);

		// 添加 m_save_path
		QDomElement save_path_node = doc.createElement("save_path");
		save_path_node.appendChild(doc.createTextNode(QString::fromStdString(m_save_path)));
		root.appendChild(save_path_node);

		// 写入文件
		QTextStream out(&file);
		doc.save(out, 4);  // 保存格式化的 XML,第二个参数表示缩进的空格数，用于控制 QDomDocument::save() 输出 XML 时的缩进格式
		file.close();

		return true;
	}
};