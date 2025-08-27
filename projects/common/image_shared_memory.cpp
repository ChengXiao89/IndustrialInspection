#include "image_shared_memory.h"
#include <QBuffer>
#include <QDebug>

image_shared_memory::image_shared_memory(const QString& key)
    : m_shared_memory(key)
{

}

bool image_shared_memory::write_image(const QImage& img, st_image_meta& meta)
{
    QImage img_rgb888 = img;
    if (img.format() != QImage::Format_RGB888) 
    {
        img_rgb888 = img.convertToFormat(QImage::Format_RGB888);
    }
    int width = img_rgb888.width();
    int height = img_rgb888.height();
	quint64 data_size = static_cast<quint64>(width * height * 3); // RGB888 每个像素 3 字节
    meta.shared_memory_key = m_shared_memory.key();
    // 需要重建共享内存
    if (!m_shared_memory.isAttached() || width != m_width || height != m_height)
    {
		meta.is_new_memory = true; // 标记为新创建的共享内存
        meta.shared_memory_key = QString("shared_image_%1x%2").arg(width).arg(height);
        m_shared_memory.setKey(meta.shared_memory_key);
        if (m_shared_memory.isAttached())
        {
            m_shared_memory.detach();
        }
        if (!m_shared_memory.create(static_cast<qsizetype>(data_size)))
        {
            qDebug() << "Shared memory create failed:" << m_shared_memory.errorString();
            return false;
        }
        m_width = width;
        m_height = height;
    }

    // 写像素数据
    if (!m_shared_memory.lock())
    {
        qDebug() << "server lock failed:" << m_shared_memory.errorString();
        return false;
    }
    memcpy(m_shared_memory.data(), img_rgb888.constBits(), static_cast<size_t>(data_size));
    m_shared_memory.unlock();

    // 填元数据
    m_frame_counter++;
    meta.width = width;
    meta.height = height;
    meta.format = QImage::Format_RGB888;
    meta.data_size = data_size;
    meta.frame_id = m_frame_counter;

    return true;
}

QImage image_shared_memory::read_image(const st_image_meta& meta)
{
    if(meta.is_new_memory || meta.shared_memory_key != m_shared_memory.key())
    {
        if (m_shared_memory.isAttached())
        {
            m_shared_memory.detach();
        }
        m_shared_memory.setKey(meta.shared_memory_key);
        if (!m_shared_memory.attach(QSharedMemory::ReadOnly))
        {
            qDebug() << "Shared memory attach failed:" << m_shared_memory.errorString();
            return QImage();
        }
    }
    else if(!m_shared_memory.isAttached())  //可能是前端重启了，此时后端没有重新创建但前端仍然需要绑定
    {
        m_shared_memory.setKey(meta.shared_memory_key);
        if (!m_shared_memory.attach(QSharedMemory::ReadOnly))
        {
            qDebug() << "Shared memory attach failed:" << m_shared_memory.errorString();
            return QImage();
        }
	}
    int width = meta.width;
    int height = meta.height;
	// 如果宽高数据异常，直接返回空图像
    if (width == 0 || height == 0)
    {
        qDebug() << "Shared memory info error...";
        return QImage();
    }
    if (!m_shared_memory.lock()) 
    {
        qDebug() << "client lock failed:" << m_shared_memory.errorString();
        return QImage();
    }
    QImage img(width, height, QImage::Format_RGB888);
    memcpy(img.bits(), m_shared_memory.constData(), static_cast<size_t>(width * height * 3));
    m_shared_memory.unlock();
    return img;
}

QJsonObject image_shared_memory::meta_to_json(const st_image_meta& meta)
{
    QJsonObject json;
    json["shm_key"] = meta.shared_memory_key;
    json["width"] = meta.width;
    json["height"] = meta.height;
    json["format"] = meta.format;
    json["is_new_memory"] = meta.is_new_memory;
    json["data_size"] = static_cast<long long>(meta.data_size);
    json["frame_id"] = static_cast<long long>(meta.frame_id);
    return json;
}

st_image_meta image_shared_memory::json_to_meta(const QJsonObject& json)
{
    st_image_meta meta;
    meta.shared_memory_key = json["shm_key"].toString();
    meta.width = json["width"].toInt();
    meta.height = json["height"].toInt();
    meta.format = json["format"].toInt();
    meta.is_new_memory = static_cast<quint64>(json["is_new_memory"].toBool());
    meta.data_size = static_cast<quint64>(json["data_size"].toDouble());
    meta.frame_id = static_cast<quint64>(json["frame_id"].toDouble());
    return meta;
}
