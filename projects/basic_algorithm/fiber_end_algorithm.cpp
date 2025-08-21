#include "fiber_end_algorithm.h"

#include <filesystem>
#include <QString>
#include <QDebug>

fiber_end_algorithm::fiber_end_algorithm()
{
	
}

bool fiber_end_algorithm::initialize(const std::string& model_path,const std::string& index_path, const std::string& shape_model_path)
{
	if (!std::filesystem::exists(index_path))
	{
		std::cerr << "Index file does not exist!" << std::endl;
		return false;
	}
	try 
	{
		m_faiss_index = faiss::read_index(index_path.c_str());
	}
	catch (const std::exception& e) 
	{
		std::cerr << "Failed to read Faiss index: " << e.what() << std::endl;
		m_faiss_index = nullptr;
		return false;
	}
	if(!std::filesystem::exists(shape_model_path))
	{
		std::cerr << "shape model file does not exist!" << std::endl;
		return false;
	}
	try
	{
		m_shape_model= shape_match::read_shape_model(shape_model_path);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to read shape model file: " << e.what() << std::endl;
		return false;
	}
	if(!m_predictor.initialize(model_path))
	{
		return false;
	}
	return true;
}

int fiber_end_algorithm::run()
{
	m_dust_boxes.clear();	//清空结果
	// 1. 寻找端面
	std::vector<st_detect_box> detect_boxes = search_fiber_end();
	if(detect_boxes.size() < 1)
	{
		return -1;		//寻找端面失败
	}
	if(DEBUG_LOCAL)
	{
		std::vector < std::vector<st_detect_box>> boxes;
		boxes.emplace_back(detect_boxes);
		draw_boxes_to_image(m_image, boxes, "../../test_images/search_fiber_end.jpg");
	}

	// 2. 形状匹配
	std::vector<st_detect_box> shape_match_boxes = get_shape_match_results(detect_boxes);
	if(shape_match_boxes.size() < 1)
	{
		return -2;		//形状匹配失败
	}
	if (DEBUG_LOCAL)
	{
		std::vector < std::vector<st_detect_box>> boxes;
		boxes.emplace_back(shape_match_boxes);
		draw_boxes_to_image(m_image, boxes, "../../test_images/shape_match_results.jpg");
	}
	// 3. 灰尘检测
	m_dust_boxes = detect_dust(shape_match_boxes);

	return 0;
}

std::vector<st_detect_box> fiber_end_algorithm::search_fiber_end()
{
    /********************* 1.缩放重采样以及预处理 *******************/
    cv::Mat scaled_image;
	const int scale_width = static_cast<int>(m_image.cols * m_scale_x);
	const int scale_height = static_cast<int>(m_image.rows * m_scale_y);
	cv::resize(m_image, scaled_image, cv::Size(scale_width, scale_height), 0.0, 0.0, cv::INTER_LINEAR);
	if(DEBUG_LOCAL)
	{
		if (!basic_algorithm::check_same_data(scaled_image, "../../test_images/test_fiber_end_resize_image.npy"))
		{
			return std::vector<st_detect_box>();
		}
	}
	//预处理，包括归一化以及通道顺序转换
	std::vector<float> mean = { 0.485f, 0.456f, 0.406f };
	std::vector<float> std = { 0.229f, 0.224f, 0.225f };
	cv::Mat normalized_image = normalize_image(scaled_image,mean,std);
	//这里得到的影像数据是HWC格式，将之转换成CHW
	normalized_image = convert_hwc_to_chw(normalized_image);
	if (DEBUG_LOCAL)
	{
		if (!basic_algorithm::check_same_data(normalized_image, "../../test_images/test_fiber_end_normalized_image.npy"))
		{
			return std::vector<st_detect_box>();
		}
	}
	/********************* 2.特征提取 *******************/
	//使用onnx模型推理，得到特征图
	st_model_output output;
	if(!m_predictor.predict(normalized_image, output))
	{
		return std::vector<st_detect_box>();	//模型推理失败
	}
	if(DEBUG_LOCAL)
	{
		if(!predictor::check_same_data(output,"../../test_images/test_fiber_end_feature_map.npy"))
		{
			return std::vector<st_detect_box>();	//推理结果异常
		}
	}
	/********************* 3.特征匹配 *******************/
	//模型推理得到的结果尺寸为是BCHW,这里得到其BC[start_x->end_x][start_y->end_y]的部分
	int start_x = 0;
	int start_y = 0;
	int end_x = int(m_scale_x * m_image.cols / m_template_scale);
	int end_y = int(m_scale_y * m_image.rows / m_template_scale);
	std::vector<float> range_output;
	int batch(0), channel(0), height(0), width(0);
	if(!get_model_output_in_range(output, start_x, start_y, end_x-1, end_y-1, 
		range_output, batch, channel, height, width))
	{
		return std::vector<st_detect_box>();		//获取范围数据失败
	}
	//滑动窗口展开，得到 feature_count 个维度为 featrue_dim 的特征向量
	std::vector<float> features;
	int feature_count_x(0), feature_count_y(0), featrue_dim(0);
	if(!extract_sliding_window_features(range_output.data(),batch,channel,height,width,m_split_window_size, m_split_window_step,
		features, feature_count_y, feature_count_x,featrue_dim))
	{
		return std::vector<st_detect_box>();		//滑动窗口展开失败
	}
	//对每个向量进行归一化
	int feature_count = feature_count_x * feature_count_y;
	normalize_vector_L2(features.data(), feature_count, featrue_dim);
	int search_count = 1;	//查找一个最邻近向量
	std::vector<faiss::idx_t> ids(feature_count * search_count);
	std::vector<float> distances(feature_count * search_count);
	m_faiss_index->search(feature_count, features.data(), search_count, distances.data(), ids.data());
	//保存符合条件的向量对应的原始矩形框
	std::vector<st_detect_box> detect_boxes;
	for(int i = 0;i < distances.size();i++)
	{
		if(static_cast<double>(distances[i]) >= m_similarity_threshold)
		{
			//distances 是一个尺寸为 feature_count 的一维数组，它记录每个特征向量与特征库中向量的最近距离
			//i 表示当前向量在一维数组中的位置，将之转换到二维数组(feature_count_x,feature_count_y)中的位置，
			//然后转换到模型推理的结果中的位置
			double coord_x = i % feature_count_x + m_split_window_size / 2.0 + start_x;
			double coord_y = i / feature_count_x + m_split_window_size / 2.0 + start_y;
			//(coord_x,coord_y) 表示符合条件(与库中特征足够接近)的特征在模型推理结果中的位置,将之变换到原始影像
			double img_x = coord_x * m_template_scale / m_scale_x;
			double img_y = coord_y * m_template_scale / m_scale_y;

			detect_boxes.emplace_back(distances[i], img_x - m_target_size_x * 0.5, img_y - m_target_size_y * 0.5,
									 img_x + m_target_size_x * 0.5, img_y + m_target_size_y * 0.5);
		}
	}
	//使用极大值抑制，去除重叠且置信度更低的检测结果
	remove_overlap_boxes(detect_boxes, m_iou_thresh);
	return detect_boxes;
}

std::vector<st_detect_box>  fiber_end_algorithm::get_shape_match_results(const std::vector<st_detect_box>& detect_boxes)
{
	std::vector<st_detect_box> shape_match_results;
	shape_match_results.reserve(detect_boxes.size());
	cv::Rect empty_rect = cv::Rect(0, 0, 0, 0);
	for (int i = 0;i < detect_boxes.size();i++)
	{
		const st_detect_box& box = detect_boxes[i];
		cv::Rect roi;
		cv::Mat gray_image = get_roi_image(m_image,box, m_shape_extent, 1, roi);
		if(DEBUG_LOCAL && 0)
		{
			cv::imwrite("../../test_images/shape_match_input_0.jpg", gray_image);
		}
		std::vector<shape_match::MatchResult> match_result = shape_match::find_shape_model(
			gray_image, m_shape_model, empty_rect, 0.5, 0.01f, 1, 10, 4);
		if(match_result.size() != 1)
		{
			continue;
		}
		shape_match_results.emplace_back(
			st_detect_box(match_result[0].similarity, match_result[0].x + roi.x, match_result[0].y + roi.y,
			match_result[0].x + match_result[0].w + roi.x, match_result[0].y + match_result[0].h + roi.y));
		if (DEBUG_LOCAL && 0)
		{
			
			st_detect_box test_result(match_result[0].similarity, match_result[0].x, match_result[0].y,
				match_result[0].x + match_result[0].w, match_result[0].y + match_result[0].h);
			std::vector<st_detect_box> test_results;
			test_results.emplace_back(test_result);
			std::vector < std::vector<st_detect_box>> boxes;
			boxes.emplace_back(test_results);
			draw_boxes_to_image(gray_image, boxes, "../../test_images/shape_match_output_0.jpg");
		}
	}
	return shape_match_results;
}


std::vector<std::vector<st_detect_box>> fiber_end_algorithm::detect_dust(const std::vector<st_detect_box>& detect_boxes)
{
	std::vector<st_detect_box> dusts_a;
	std::vector<st_detect_box> dusts_b;
	for (int i = 0;i < detect_boxes.size();i++)
	{
		detect_dust(detect_boxes[i], dusts_a, dusts_b);
	}
	std::vector<std::vector<st_detect_box>> dusts;
	dusts.emplace_back(dusts_a);
	dusts.emplace_back(dusts_b);
	return dusts;
}

bool fiber_end_algorithm::detect_dust(const st_detect_box& detect_box, std::vector<st_detect_box>& dusts_a, std::vector<st_detect_box>& dusts_b)
{
	cv::Rect roi;
	cv::Mat image = get_roi_image(m_image,detect_box, m_dust_extent, 1, roi);
	if(DEBUG_LOCAL)
	{
		cv::imwrite("../../test_images/cropped_img_0.jpg",image);
	}
	//均值滤波
	cv::Mat mean_image;
	cv::blur(image, mean_image, cv::Size(m_dust_mean_kernel_size, m_dust_mean_kernel_size));
	//进行偏移对比，然后得到单通道二值图
	cv::Mat mean_offset = mean_image - cv::Scalar(m_dust_mean_image_offset);
	cv::Mat mask;
	cv::compare(image, mean_offset, mask, cv::CMP_LT);
	//创建一个与image同尺寸的单通道二值图，以其中心为圆心，B 类灰尘半径画圆，圆为白色
	cv::Point center(image.cols / 2, image.rows / 2);
	cv::Mat zero_img = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::circle(zero_img, center, m_dust_radius_b, cv::Scalar(255), cv::FILLED);
	// 计算 mask_gray & zero_img，将 mask_gray 中落在 zero_img 圆形范围内的非 0 像素保留，其它全部置为0，
	// 结果保存在 dust_img 中
	cv::Mat dust_img;
	cv::bitwise_and(mask, zero_img, dust_img);
	//寻找边界，得到检测结果
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(dust_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


	//面积筛选,只保留面积大于阈值的检测结果
	//if(m_dust_min_area > 0.00001)	//cv计算边界的面积可以为0，因此即使输入阈值为0，也需要进行筛选
	{
		std::vector<std::vector<cv::Point>> results;
		results.reserve(contours.size());
		for (int i = 0;i < contours.size();i++)
		{
			std::vector<cv::Point>& contour = contours[i];
			if(fabs(cv::contourArea(contour)) > m_dust_min_area)
			{
				results.emplace_back(contour);
			}
		}
		contours = std::move(results);
	}
	//结果分类，如果检测结果包围盒与以影像中心为圆心、m_dust_radius_a为半径的圆相交，将之划分到 a 类灰尘
	//否则划分到 b 类灰尘，这里计算包围盒四个顶点到影像中心的最小距离，然后判断即可
	for (int i = 0; i < contours.size(); i++)
	{
		std::vector<cv::Point>& contour = contours[i];
		cv::Rect rect = cv::boundingRect(contour);
		if(is_circle_rect_intersect(center, m_dust_radius_a, rect))
		{
			dusts_a.emplace_back(st_detect_box(0.0, rect.x + roi.x, rect.y + roi.y,
				rect.x + rect.width + roi.x, rect.y + rect.height + roi.y));
		}
		else
		{
			dusts_b.emplace_back(st_detect_box(0.0, rect.x + roi.x, rect.y + roi.y,
				rect.x + rect.width + roi.x, rect.y + rect.height + roi.y));
		}
	}
	return true;
}


void fiber_end_algorithm::remove_overlap_boxes(std::vector<st_detect_box>& detect_boxes, double iou_thresh)
{
	if (detect_boxes.empty())
	{
		return;
	}
	std::sort(detect_boxes.begin(), detect_boxes.end()); // 按照分数从大到小排序
	std::vector<st_detect_box> result;
	result.reserve(detect_boxes.size());
	for (size_t i = 0; i < detect_boxes.size(); ++i)
	{
		bool keep = true;
		for (const auto& box : result)
		{
			if (get_iou(detect_boxes[i], box) > iou_thresh)
			{
				keep = false;
				break;
			}
		}
		if (keep)
		{
			result.push_back(detect_boxes[i]);
		}
	}
	detect_boxes = std::move(result);
}