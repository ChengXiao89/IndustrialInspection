// test_auto_focus.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include "motion_control.h"
#include "../device_camera/interface_camera.h"

int calcu_focus_val(const cv::Mat& input_img, const cv::Rect& rect);

class AutoFocus
{
 public:
	AutoFocus(motion_control* mc, interface_camera* camera,int fiber_end_count);
	~AutoFocus() = default;

  int fiber_nums = 8;               // 光纤端面数目，默认为 8
  void calibrate_model(int n = 0);  // 标定
  void save_model(const std::string& filename);
  bool load_model(const std::string& filename);
  int search_direction(int fiber_num);

  void coarse_localization(int fiber_num = 0);  // 粗定位
  bool fine_localization(int fiber_num = 0);    // 细定位
  std::vector<cv::Mat> get_focus_images();  // 获取指定数量的端面清晰图像

  cv::Mat get_image();

  void set_camera(interface_camera* camera) { m_camera = camera; }
  interface_camera* get_camera() { return m_camera; }
 private:
  motion_control* mc_{nullptr};      // 引用成员，保存传入的 motion_control
  interface_camera* m_camera;
  int direction = 0;        // 0正向移动，1负向移动
  std::vector<int> focus_val_model;  // 保存标定的焦距值
  int calibrate_step = 5;            // 标定时的步长
  int calibrate_speed = 200;         // 标定时的速度
  int image_height = 0;              // 保存图像高度
  int image_width = 0;               // 保存图像宽度
  int model_max_val = 0;             // 保存模型最大值
  int model_max_idx = 0;             // 保存模型最大值的索引
};