/*************************************************
 * 算法库工程.
 * 包括一些基础算法以及一些功能算法
 * 基础算法指的是基本的影像处理，例如影像缩放、灰度变换等
 * 功能算法指的是一些功能API，例如端面检测、灰尘检测等
 **************************************************/

#pragma once
#include "basic_algorithm_global.h"
#include "common_api.h"

const int DEBUG_LOCAL = 0;

/*************************************
 * 算法基类，常见的图像处理算法，基于 OpenCV 实现
 *************************************/
class BASIC_ALGORITHM_EXPORT basic_algorithm
{
public:
    basic_algorithm() = default;
    virtual  ~basic_algorithm() = default;

    virtual int run() = 0;          //处理接口

    virtual void set_data(const cv::Mat& image) { m_image = image; }

    // 测试接口: 检测数据是否与参考数据一致,参考数据需要保存为 float 类型
    static bool check_same_data(const cv::Mat& data, const std::string& ref_data_path);
public:
    cv::Mat m_image;
};
