#pragma once
#include <opencv2/opencv.hpp>

#include "basic_algorithm_global.h"

#include "predictor.h"

//检测结果包围盒
struct st_detect_box
{
    double m_score{ 0.0 };
    double m_x0{ 0.0 }, m_y0{ 0.0 }, m_x1{ 0.0 }, m_y1{ 0.0 };

    st_detect_box(double score = 0.0, double x0 = 0.0, double y0 = 0.0, double x1 = 0.0, double y1 = 0.0)
        :m_score(score), m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1)
    {
    }

    //重载运算符，使 st_detect_box 支持 std::sort 按照分数从大到小排序
    bool operator<(const st_detect_box& other) const
    {
        return m_score > other.m_score; // 注意这里是 >，保证 std::sort 从大到小
    }
};

//HWC-->CHW
cv::Mat BASIC_ALGORITHM_EXPORT convert_hwc_to_chw(const cv::Mat& src);

//CHW-->HWC
cv::Mat BASIC_ALGORITHM_EXPORT convert_chw_to_hwc(const cv::Mat& src);

//归一化处理.处理HWC格式影像
cv::Mat BASIC_ALGORITHM_EXPORT normalize_image(const cv::Mat& src, std::vector<float> mean = std::vector<float>(), std::vector<float> std = std::vector<float>());


/************************************************
 * 模型输出结果 model_output 的尺寸为B*C*H*W,在 H 与 W 维度截取其[start_x,start_y]-->[end_x,end_y] 部分的数据
 * 结果存储在 output 中,尺寸为 batch*channel*width*height
 * 其中 batch 与 channel 与原始尺寸一致
 *************************************************/
bool BASIC_ALGORITHM_EXPORT get_model_output_in_range(const st_model_output& model_output, int start_x, int start_y, int end_x, int end_y,
	std::vector<float>& output, int& batch, int& channel, int& height, int& width);

/************************************************
 * 将模型输出的结果进行滑动窗口展开，然后转换成特征向量
 * const float* data,int batch,int channel,int width,int height -- 模型输出的结果以及尺寸
 * kernel_size --- 窗口尺寸,
 * step_size   --- 滑动步长
 * 结果存储在 features 中,特征向量个数为count，每个向量维度为dims
 *************************************************/
bool BASIC_ALGORITHM_EXPORT extract_sliding_window_features(const float* data,int batch,int channel, int height, int width, int kernel_size,int step_size,
	std::vector<float>& features, int& feature_count_y, int& feature_count_x, int& dims);

//对向量进行 L2 归一化，向量个数为 count，维度为 dims.
void BASIC_ALGORITHM_EXPORT normalize_vector_L2(float* input, int count, int dims);

//计算两个检测框的交并比
double BASIC_ALGORITHM_EXPORT get_iou(const st_detect_box& box1, const st_detect_box& box2);

//判断矩形与圆是否相交
bool BASIC_ALGORITHM_EXPORT is_circle_rect_intersect(const cv::Point& center, double radius, const cv::Rect& rect);

//获取指定范围影像数据，可指定外扩距离以及通道数,同时返回新图像在原始图像上的范围.
//输入的影像是三通道但各个通道完全相同
cv::Mat BASIC_ALGORITHM_EXPORT get_roi_image(const cv::Mat& image, const st_detect_box& box, int extent, int channel, cv::Rect& roi);

//将包围盒绘制在影像上并保存到文件
void BASIC_ALGORITHM_EXPORT draw_boxes_to_image(const cv::Mat& image, const std::vector<std::vector<st_detect_box>>& boxes, const std::string& file_path);

//将包围盒绘制在影像上并返回绘制结果
cv::Mat BASIC_ALGORITHM_EXPORT draw_boxes_to_image(const cv::Mat& image, const std::vector<std::vector<st_detect_box>>& boxes);