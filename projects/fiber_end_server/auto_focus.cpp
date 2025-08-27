#include "auto_focus.h"

cv::Mat QImage_to_cvMat(const QImage& image)
{
    switch (image.format())
    {
    case QImage::Format_RGB888:
        return cv::Mat(image.height(), image.width(), CV_8UC3,
            (void*)image.constBits(), image.bytesPerLine()).clone();

    case QImage::Format_Grayscale8:
        return cv::Mat(image.height(), image.width(), CV_8UC1,
            (void*)image.constBits(), image.bytesPerLine()).clone();

    default:
        // 其他格式先转为 RGB888
        QImage converted = image.convertToFormat(QImage::Format_RGB888);
        return cv::Mat(converted.height(), converted.width(), CV_8UC3,
            (void*)converted.constBits(), converted.bytesPerLine()).clone();
    }
}

int calcu_focus_val(const cv::Mat& input_img, const cv::Rect& rect) {
  // 输入必须是灰度图
  cv::Mat img;
  if (rect.width > 0 && rect.height > 0) {
    // 提取矩形区域
    img = input_img(rect).clone();  // clone 创建独立副本
  } else {
    img = input_img;
  }

  // 使用 Sobel 算子计算梯度
  cv::Mat sobel_x, sobel_y;
  cv::Sobel(img, sobel_x, CV_32F, 1, 0, 3);
  cv::Sobel(img, sobel_y, CV_32F, 0, 1, 3);

  // 计算梯度平方和
  cv::Mat grad_sq = sobel_x.mul(sobel_x) + sobel_y.mul(sobel_y);

  // 计算平均值
  double mean_val = cv::mean(grad_sq)[0];

  // 仅保留强边缘
  cv::Mat mask = grad_sq > mean_val;
  grad_sq.setTo(0, ~mask);

  // 计算总和并缩放
  double sum_val = cv::sum(grad_sq)[0];

  return static_cast<int>(sum_val / 1e5);
}

AutoFocus::AutoFocus(motion_control* mc, interface_camera* camera, int fiber_end_count)
    : mc_(mc), m_camera(camera), fiber_nums(fiber_end_count)
{
    load_model("./calibration.bin");
}

cv::Mat AutoFocus::get_image()
{
	if(m_camera != nullptr)
	{
        if(1)
        {
            m_camera->set_trigger_mode(global_trigger_mode_once);
            m_camera->set_trigger_source(global_trigger_source_software);
            m_camera->start_grab();
        }
        QImage img = m_camera->trigger_once();
        return QImage_to_cvMat(img.convertToFormat(QImage::Format_Grayscale8));
	}
    return cv::Mat();
}

void AutoFocus::save_model(const std::string& filename) {
  if (focus_val_model.size() == 0) {
    std::cout << "please calibrate model first" << std::endl;
    return;
  }
  std::ofstream fout(filename, std::ios::binary);
  if (!fout) {
    std::cerr << "Failed to open file for writing: " << filename << std::endl;
    return;
  }
  size_t n = focus_val_model.size();
  fout.write(reinterpret_cast<const char*>(&n), sizeof(n));  // 保存大小
  fout.write(reinterpret_cast<const char*>(focus_val_model.data()),
             n * sizeof(int));  // 保存数据
};

bool AutoFocus::load_model(const std::string& filename)
{
  std::ifstream fin(filename, std::ios::binary);
  if (!fin) 
  {
    std::cerr << "Failed to open file for reading: " << filename << std::endl;
    return false;
  }
  focus_val_model.clear();
  size_t n;
  fin.read(reinterpret_cast<char*>(&n), sizeof(n));  // 读取大小
  focus_val_model.resize(n);
  fin.read(reinterpret_cast<char*>(focus_val_model.data()), n * sizeof(int));
  auto it = std::max_element(focus_val_model.begin(), focus_val_model.end());
  model_max_val = *it;
  model_max_idx = std::distance(focus_val_model.begin(), it);
    return true;
};

/*
 * @brief 标定模型
 * @param n 轴号，默认为 0
 * @note
 * 先手动调整到达最清晰位置，在该位置前后50*calibrate_step个脉冲标定清晰度曲线
 */
void AutoFocus::calibrate_model(int n)
{
  mc_->move_distance(n, -50*calibrate_step, calibrate_speed);
  cv::Mat img = get_image();
  int height = img.rows;
  int width = img.cols;

  int calu_width = width / fiber_nums;
  // 标定
  focus_val_model.clear();
  for (int i = 0; i < 100; i++) {
    mc_->move_distance(n, calibrate_step, calibrate_speed);
    int value = calcu_focus_val(get_image(), cv::Rect(0, 0, calu_width, height));
    std::cout << value << std::endl;
    focus_val_model.push_back(value);
  }
  auto it = std::max_element(focus_val_model.begin(), focus_val_model.end());
  model_max_val = *it;
  model_max_idx = std::distance(focus_val_model.begin(), it);
  /*save_model("focus_model.bin");*/
};

/*
 * @brief 确定移动方向
 * @param 端面号，默认为
 * 0，从左到右为0~fiber_num-1
 */
int AutoFocus::search_direction(int fiber_num)
{
  if (fiber_num < 0 || fiber_num >= fiber_nums) 
  {
    std::cerr << "Invalid fiber number: " << fiber_num << std::endl;
    return -1;
  }
  cv::Mat img = get_image();
  image_height = img.rows;
  image_width = img.cols;
  int calu_width = image_width / fiber_nums;
  int focus_val = calcu_focus_val(
      get_image(),
      cv::Rect(fiber_num * calu_width, 0, calu_width, image_height));
  // 如果清晰度大于某一阈值，步长设置为小值，否则设置为大值
  if (focus_val > model_max_val * 0.8) {
    mc_->move_distance(0, 2, 200);
    int focus_val_f = calcu_focus_val(
        get_image(),
        cv::Rect(fiber_num * calu_width, 0, calu_width, image_height));
    if (focus_val_f > focus_val) {
      direction = 0;  // 正向移动
    } else {
      direction = 1;  // 负向移动
    }
    return focus_val;
  }
  else if (focus_val > model_max_val * 0.5) {
      mc_->move_distance(0, 5, 200);
      int focus_val_f = calcu_focus_val(
          get_image(),
          cv::Rect(fiber_num * calu_width, 0, calu_width, image_height));
      if (focus_val_f > focus_val + 20) {
          mc_->move_distance(0, -3, 200);
          direction = 0;  // 正向移动
      }
      else {
          mc_->move_distance(0, -5, 200);
          direction = 1;  // 负向移动
      }
      return focus_val;
  }
  else if (focus_val > model_max_val * 0.3) {
    mc_->move_distance(0, 10, 200);
    int focus_val_f = calcu_focus_val(
        get_image(),
        cv::Rect(fiber_num * calu_width, 0, calu_width, image_height));
    if (focus_val_f > focus_val+20) {
        mc_->move_distance(0, -5, 200);
      direction = 0;  // 正向移动
    } else {
        mc_->move_distance(0, -10, 200);
      direction = 1;  // 负向移动
    }
    return focus_val;
  }
  else
  {
      mc_->move_distance(0, 20, 200);
      int focus_val_f = calcu_focus_val(
          get_image(),
          cv::Rect(fiber_num * calu_width, 0, calu_width, image_height));
      if (focus_val_f > focus_val + 40) {
          direction = 0;  // 正向移动
          mc_->move_distance(0, -10, 200);
      }
      else {
          mc_->move_distance(0, -20, 200);
          direction = 1;  // 负向移动
      }
      return focus_val;
  }
};

void AutoFocus::coarse_localization(int fiber_num) {
  if (focus_val_model.size() == 0) {
    std::cout << "please calibrate model or load model first" << std::endl;
    return;
  }
  int curren_val = search_direction(fiber_num);
  std::cout << curren_val << std::endl;
  int min_idx = 0;
  int min_diff = std::abs(curren_val - focus_val_model[0]);
  for (int i = 0; i < focus_val_model.size(); i++) {
    int diff = std::abs(curren_val - focus_val_model[i]);
    if (diff < min_diff) {
      min_diff = diff;
      min_idx = i;
    }
  }
  int move_distance = std::abs(model_max_idx - min_idx) * calibrate_step;
  if (direction == 0) {
    mc_->move_distance(0, move_distance * 0.9, 1000);
  } else {
    mc_->move_distance(0, -move_distance * 0.9, 1000);
  }
};

bool AutoFocus::fine_localization(int fiber_num) {
  if (focus_val_model.size() == 0) {
    std::cout << "please calibrate model or load model first" << std::endl;
    return false;
  }
  coarse_localization(fiber_num);
  int step;
  if (direction == 0)
    step = 2;
  else
    step = -2;
  int step2 = step;
  int count = 0;
  bool flag = true;
  cv::Mat img = get_image();
  int height = img.rows;
  int width = img.cols;
  int calu_width = width / fiber_nums;
  int current_val = calcu_focus_val(
      get_image(), cv::Rect(fiber_num * calu_width, 0, calu_width, height));

  for (int i = 0; i < 50; i++) 
  {
    if (current_val < model_max_val * 0.5)
      step2 = step * 4;
    else if (current_val < model_max_val * 0.7)
      step2 = step * 2;
    else
      step2 = step;
    mc_->move_distance(0, step2, 2000);
    int focus_val = calcu_focus_val(
        get_image(), cv::Rect(fiber_num * calu_width, 0, calu_width, height));
    std::cout << focus_val << std::endl;
    if (focus_val > current_val) {
      current_val = focus_val;
      flag = false;
    } else {
      count++;
      if (count > 0)
        break;
    }
  }
  if (flag) 
  {
    std::cout << "back" << std::endl;
    mc_->move_distance(0, -step2, 1000);
    current_val = calcu_focus_val(
        get_image(), cv::Rect(fiber_num * calu_width, 0, calu_width, height));
    step = -step;
    for (int i = 0; i < 50; i++) 
    {
      if (current_val < model_max_val * 0.5)
        step2 = step * 4;
      else if (current_val < model_max_val * 0.7)
        step2 = step * 2;
      else
        step2 = step;
      mc_->move_distance(0, step2, 2000);
      int focus_val = calcu_focus_val(
          get_image(),
          cv::Rect(fiber_num * calu_width, 0, calu_width, height));
      std::cout << focus_val << std::endl;
      if (focus_val >= current_val) {
        current_val = focus_val;
      } else {
        break;
      }
    }
    mc_->move_distance(0, -step2, 1000);
  }
  std::cout << "........................" << std::endl;
  std::cout << current_val << std::endl;
    return true;
}

std::vector<cv::Mat> AutoFocus::get_focus_images()
{
    if(!fine_localization())
    {
        return std::vector<cv::Mat>();
    }
  std::vector<cv::Mat> focus_images;
  focus_images.reserve(fiber_nums);
  int calu_width = image_width / fiber_nums;
  //fine_localization(0);
  cv::Mat img = get_image();
    if(1)
    {
        cv::imwrite("C:/Temp/location.png",img);
    }
  focus_images.push_back(img(cv::Rect(0, 0, calu_width, image_height)).clone());
  // 获取第fiber_num个焦点图
  int current_val = search_direction(fiber_nums - 1);
  int step;
  if (direction == 0) {
    step = 2;
  } else {
    step = -2;
  }
  std::vector<bool> flag_list = {false};
  for (int i = 1; i < fiber_nums; i++) {
    flag_list.push_back(true);
  }
  std::vector<int> last_val_list;
  std::vector<int> current_val_list;
  current_val_list.resize(fiber_nums);
  last_val_list.resize(fiber_nums);
  cv::Mat last_img = get_image();
  for (int i = 0; i < fiber_nums; i++) {
    if (flag_list[i]) {
      last_val_list[i] = calcu_focus_val(
          last_img, cv::Rect(i * calu_width, 0, calu_width, image_height));
      current_val_list[i] = last_val_list[i];
    } else {
      last_val_list[i] = 0;
      current_val_list[i] = 0;
    }
    std::cout << "-------------0" <<":" << last_val_list[i] << std::endl;
  }
  /*cv::Mat current_img = last_img.clone();*/
  for (int n = 0; n < 50; n++) {
    mc_->move_distance(0, step, 200);
    cv::Mat current_img = get_image();
    for (int i = 0; i < fiber_nums; i++) {
      if (flag_list[i]) {
        current_val_list[i] = calcu_focus_val(
            current_img, cv::Rect(i * calu_width, 0, calu_width, image_height));
        if (current_val_list[i] < last_val_list[i]) {
          flag_list[i] = false;
          focus_images.push_back(
              last_img(cv::Rect(i * calu_width, 0, calu_width, image_height))
                  .clone());
          std::cout <<"------------1" << i << ":" << current_val_list[i] << std::endl;
        }
        else
        {
            last_val_list[i] = current_val_list[i];
        }
      }
    }
    last_img = current_img.clone();
    if (std::none_of(flag_list.begin(), flag_list.end(),
                     [](bool flag) { return flag; })) {
      break;
    }
  }
  return focus_images;
}