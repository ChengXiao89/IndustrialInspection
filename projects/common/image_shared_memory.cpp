#include "image_shared_memory.h"
#include <QBuffer>
#include <QDebug>

image_shared_memory::image_shared_memory(const QString& key)
    : m_key_prefix(key)
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
    // 需要重建共享内存
    if (!m_buffers[0].isAttached() || width != m_width || height != m_height)
    {
        for (int i = 0; i < 2; ++i)
        {
            //key = 前缀_序号_width*height
            QString key = QString("%1_buffer%2_%3x%4").arg(m_key_prefix).arg(i).arg(width).arg(height);
            if (m_buffers[i].isAttached())
                m_buffers[i].detach();
            m_buffers[i].setKey(key);
            if (!m_buffers[i].create(static_cast<qsizetype>(data_size)))
            {
                qDebug() << "Shared memory create failed:" << m_buffers[i].errorString();
                return false;
            }
        }
        meta.is_new_memory = true; // 标记为新创建的共享内存
        m_width = width;
        m_height = height;
    }
    // 上一次写入的缓冲区索引是 m_index，这里切换缓冲区写入
    int write_index = 1 - m_index;
    if (!m_buffers[write_index].lock())
    {
        qDebug() << "Shared memory lock failed:" << m_buffers[write_index].errorString();
        return false;
    }
    memcpy(m_buffers[write_index].data(), img_rgb888.constBits(), static_cast<size_t>(data_size));
    m_buffers[write_index].unlock();

    // 更新元数据
    m_frame_counter++;
    meta.width = width;
    meta.height = height;
    meta.index = write_index;       //当前写入的缓冲区索引，读的时候使用该索引对应的缓冲区
    meta.shared_memory_key = m_buffers[write_index].key();
    meta.frame_id = m_frame_counter;
    // 记录当前写入的缓冲区索引，下次写入时的索引为  1 - m_index;
    m_index = write_index;
    return true;
}

QImage image_shared_memory::read_image(const st_image_meta& meta)
{
    int read_index = meta.index; // 读取的缓冲区索引
    if (meta.is_new_memory || meta.shared_memory_key != m_buffers[read_index].key())
    {
        if (m_buffers[read_index].isAttached())
            m_buffers[read_index].detach();
        m_buffers[read_index].setKey(meta.shared_memory_key);
        if (!m_buffers[read_index].attach(QSharedMemory::ReadOnly))
        {
            qDebug() << "Shared memory attach failed:" << m_buffers[read_index].errorString();
            return QImage();
        }
    }
    else if(!m_buffers[read_index].isAttached())  //可能是前端重启了，此时后端没有重新创建但前端仍然需要绑定
    {
        m_buffers[read_index].setKey(meta.shared_memory_key);
        if (!m_buffers[read_index].attach(QSharedMemory::ReadOnly))
        {
            qDebug() << "Shared memory attach failed:" << m_buffers[read_index].errorString();
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
    if (!m_buffers[read_index].lock())
    {
        qDebug() << "Shared memory lock failed:" << m_buffers[read_index].errorString();
        return QImage();
    }
    QImage img(width, height, QImage::Format_RGB888);
    memcpy(img.bits(), m_buffers[read_index].constData(), static_cast<size_t>(width * height * 3));
    m_buffers[read_index].unlock();
    return img;
}

QJsonObject image_shared_memory::meta_to_json(const st_image_meta& meta)
{
    QJsonObject json;
    json["shm_key"] = meta.shared_memory_key;
    json["width"] = meta.width;
    json["height"] = meta.height;
    json["index"] = meta.index;
    json["format"] = meta.format;
    json["is_new_memory"] = meta.is_new_memory;
    json["frame_id"] = static_cast<long long>(meta.frame_id);
    return json;
}

st_image_meta image_shared_memory::json_to_meta(const QJsonObject& json)
{
    st_image_meta meta;
    meta.shared_memory_key = json["shm_key"].toString();
    meta.width = json["width"].toInt();
    meta.height = json["height"].toInt();
	meta.index = json["index"].toInt();
    meta.format = json["format"].toInt();
    meta.is_new_memory = static_cast<quint64>(json["is_new_memory"].toBool());
    meta.frame_id = static_cast<quint64>(json["frame_id"].toDouble());
    return meta;
}
