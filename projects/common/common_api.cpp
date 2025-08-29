#include "common_api.h"

QImage convert_cvmat_to_qimage(const cv::Mat& image, int output_channels)
{
    if (image.empty() || (output_channels != 1 && output_channels != 3))
    {
        return QImage();
    }
    // 处理输入通道数和输出通道数的关系
    if (image.type() == CV_8UC1)        // 输入单通道
    {
        if (output_channels == 1)       // 输出也是单通道
        {
            return QImage(image.data, image.cols, image.rows,image.step, QImage::Format_Grayscale8).copy();
        }
        else            // 输出三通道
        {
            cv::Mat converted;
            cv::cvtColor(image, converted, cv::COLOR_GRAY2RGB);
            return QImage(converted.data, converted.cols, converted.rows,converted.step, QImage::Format_RGB888).copy();
        }
    }
    else if (image.type() == CV_8UC3)       // 输入三通道
    { 
        if (output_channels == 1)         // 输出单通道
        {
            cv::Mat converted;
            cv::cvtColor(image, converted, cv::COLOR_BGR2GRAY); // 或 COLOR_RGB2GRAY
            return QImage(converted.data, converted.cols, converted.rows,converted.step, QImage::Format_Grayscale8).copy();
        }
        else             // 输出也是三通道
        {
            // OpenCV 默认是 BGR，需要转换成 RGB
            cv::Mat converted;
            cv::cvtColor(image, converted, cv::COLOR_BGR2RGB);
            return QImage(converted.data, converted.cols, converted.rows,converted.step, QImage::Format_RGB888).copy();
        }
    }
	return QImage();
}
