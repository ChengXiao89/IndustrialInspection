/***********************************************
 * ���ض����������ļ�����������ز���.
 * Ŀǰ���� Y��λ���б�ÿ��Ӱ����������� Ӱ�񱣴�·��
 ***********************************************/

#pragma once
#include <vector>
#include <string>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

 // ���������ò���
struct st_config_data
{
	std::vector<int> m_position_list;					//Y ���ϵ�λ���б���Y��˵���Ϊ��㣬����״̬�£��������ڴ�λ�öԽ�-���
	int m_count{ 4 };									//ÿ��Ӱ���ϵĶ�������. �Զ��Խ��Լ����������Ҫʹ�õĲ���
	int m_auto_detect{ 1 };								//�Խ����֮���Ƿ��Զ�ִ�м�� 0 -- ��    1 -- ��
	std::string m_save_path{ "./saveimages" };		//ָ����������ͼ���·��

	std::string m_config_file_path{ "./config.xml" };		//�����ļ�·��,���������֮�����������ļ���ֻ�ڵ��� load_from_file ʱ��ʼ��һ��

	//�����������ʱ����������ļ��������ļ�·���Լ�����
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
		// ��ȡ position_list
		QDomNodeList positions_node = root.elementsByTagName("positions");
		for (int i = 0; i < positions_node.count(); ++i) 
		{
			QDomElement elem = positions_node.at(i).toElement();
			m_position_list.push_back(elem.text().toInt());
		}
		// ��ȡ m_count
		QDomNode count_node = root.namedItem("count");
		if (!count_node.isNull()) 
		{
			m_count = count_node.toElement().text().toInt();
		}
		// ��ȡ m_auto_detect
		QDomNode auto_detect_node = root.namedItem("auto_detect");
		if (!auto_detect_node.isNull())
		{
			m_auto_detect = auto_detect_node.toElement().text().toInt();
		}
		// ��ȡ m_save_path
		QDomNode save_path_node = root.namedItem("save_path");
		if (!save_path_node.isNull()) 
		{
			m_save_path = save_path_node.toElement().text().toStdString();
		}
		return true;
	}

	//�������޸��������֮��������ݣ�Ȼ�󱣴浽�ļ�
	bool save() const
	{
		QFile file(QString::fromStdString(m_config_file_path));
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) 
		{
			qDebug() << "Failed to open config file for writing";
			return false;
		}
		QDomDocument doc;
		// �������ڵ�
		QDomElement root = doc.createElement("config");
		doc.appendChild(root);
		// ���� m_position_list
		for (int pos : m_position_list) 
		{
			QDomElement position_node = doc.createElement("position");
			position_node.appendChild(doc.createTextNode(QString::number(pos)));
			root.appendChild(position_node);
		}
		// ���� m_count
		QDomElement count_node = doc.createElement("count");
		count_node.appendChild(doc.createTextNode(QString::number(m_count)));
		root.appendChild(count_node);

		// ���� m_auto_detect
		QDomElement auto_detect_node = doc.createElement("auto_detect");
		auto_detect_node.appendChild(doc.createTextNode(QString::number(m_auto_detect)));
		root.appendChild(auto_detect_node);

		// ��� m_save_path
		QDomElement save_path_node = doc.createElement("save_path");
		save_path_node.appendChild(doc.createTextNode(QString::fromStdString(m_save_path)));
		root.appendChild(save_path_node);

		// д���ļ�
		QTextStream out(&file);
		doc.save(out, 4);  // �����ʽ���� XML,�ڶ���������ʾ�����Ŀո��������ڿ��� QDomDocument::save() ��� XML ʱ��������ʽ
		file.close();

		return true;
	}
};