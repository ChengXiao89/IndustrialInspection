// test_auto_focus.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include "motion_control.h"
#include "../device_camera/interface_camera.h"

int calcu_focus_val(const cv::Mat& input_img, const cv::Rect& rect=cv::Rect(0,0,0,0));

class auto_focus
{
 public:
	 auto_focus(void* operate_obj);		//调用自动对焦模块的对象
	~auto_focus() = default;

	
	void calibrate_model(int n = 0);  // 标定
	void save_model(const std::string& filename);
	bool load_model(const std::string& filename);

	int search_direction(int start_index);
	void coarse_localization(int start_index = 0);  // 粗定位,从整张影像的第几个端面开始定位
	bool fine_localization(int start_index = 0);    // 细定位,从整张影像的第几个端面开始定位
	std::vector<cv::Mat> get_focus_images();		// 获取指定数量的端面清晰图像

	cv::Mat get_image();

 private:
	void* m_operate_object{ nullptr };		//调用自动对焦模块的对象指针

	int direction = 0;					// 0正向移动，1负向移动
	std::vector<int> focus_val_model;	// 保存标定的焦距值
	int calibrate_step = 2;				// 标定时的步长
	int calibrate_speed = 1000;         // 标定时的速度
	int image_height = 0;				// 保存图像高度
	int image_width = 0;				// 保存图像宽度
	int model_max_val = 0;				// 保存模型最大值
	int model_max_idx = 0;				// 保存模型最大值的索引
};