/*********************************
 * 端面检测算法
 *******************************/
#pragma once

#include "basic_algorithm.h"
#include <vector>

#include "predictor.h"
#include "shape_match.h"
#include "faiss/index_io.h"
#include "faiss/IndexFlat.h"



class BASIC_ALGORITHM_EXPORT fiber_end_algorithm : public basic_algorithm
{
public:
	/********************************
	 * 端面检测，相机在灰度模式下拍摄，外部将之转换成三通道 RGB 格式
	 * 这里的影像数据是三通道灰度图像
	 ********************************/
	fiber_end_algorithm();
	virtual ~fiber_end_algorithm() = default;

	bool initialize(const std::string& model_path, const std::string& index_path,const std::string& shape_model_path);

	virtual int run() override;


	//粗定位寻找端面 
	std::vector<st_detect_box> search_fiber_end();

	//在粗定位寻找端面之后, 形状匹配得到更精确的结果,输入参数为形状匹配的结果，都是原始影像坐标
	std::vector<st_detect_box>  get_shape_match_results(const std::vector<st_detect_box>&detect_boxes);

	//灰尘检测. 在形状匹配之后进行灰尘检测得到最终结果,需要返回两种灰尘结果，因此这里返回二维数组
	std::vector<std::vector<st_detect_box>> detect_dust(const std::vector<st_detect_box>& detect_boxes);
	bool detect_dust(const st_detect_box& detect_box, std::vector<st_detect_box>& dusts_a, std::vector<st_detect_box>& dusts_b);

	//获取检测结果
	const std::vector<std::vector<st_detect_box>>& result() const { return m_dust_boxes; }

	//取分数最高的矩形框，同时移除与之重叠度大于阈值的其他矩形框
	static void remove_overlap_boxes(std::vector<st_detect_box>& detect_boxes,double iou_thresh = 0.2);

private:
	cv::Mat m_gray_image;
	double m_scale_x{ 0.1 };					//原始影像缩放系数，缩放之后传给resnet18网络提取特征
	double m_scale_y{ 0.1 };
	double m_target_size_x{ 589.0 };			//端面查找结果的矩形框尺寸
	double m_target_size_y{ 554.5 };
	int m_template_scale{ 16 };					// 特征模板缩放系数，在对原始影像缩放(供网络处理)之后，继续进行缩放使其与特征图对应
	int m_split_window_size{ 3 };				// 滑动窗口大小
	int m_split_window_step{ 1 };				// 滑动窗口步长
	double m_similarity_threshold{ 0.7 };		// 特征匹配相似度阈值
	double m_iou_thresh{ 0.2 };					// 非极大值抑制时交并比阈值
	int m_shape_extent{ 120 };					// 在模型检测到端面之后，需要进行形状匹配，指定对端面外包盒的外扩距离
	int m_dust_extent{ 50 };					// 形状匹配之后，对检测结果包围盒进行外扩，以进行灰尘检测
	int m_dust_radius_a{ 130 };					// a 类灰尘半径阈值，如果灰尘检测结果与该半径的中心圆相交，划分到 a 类
	int m_dust_radius_b{ 260 };					// b 类灰尘半径阈值，如果灰尘检测结果与该半径的中心圆相交，划分到 b 类
	int m_dust_mean_kernel_size{ 15 };			// 灰尘检测时需要做局部均值处理，这里指定窗口大小
	int m_dust_mean_image_offset{ 5 };			// 进行局部均值处理之后，得到偏移之后的二值图，这里指定偏移量
												// 如果原始像素小于均值图中像素该偏移量，置为255
	double m_dust_min_area{ 0.0 };				// 最小面积阈值，在提取出(灰尘)轮廓之后，如果面积小于该值则移除
												// 这里使用 0.0 表示保留所有的检测结果
	std::vector<std::vector<st_detect_box>> m_dust_boxes;	//灰尘检测结果
	faiss::Index* m_faiss_index{ nullptr };
	shape_match::TemplData m_shape_model;
	predictor m_predictor;
};
