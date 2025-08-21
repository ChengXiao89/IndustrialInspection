#include "shape_match.h"
#include <fstream>
#include <stdexcept>

namespace {
namespace utils {

void Quantify16(cv::Mat& angle,
                cv::Mat& quantized_angle,
                cv::Mat& mag,
                float mag_thresh);
void Quantify180(cv::Mat angle,
                 cv::Mat& quantized_angle,
                 cv::Mat mag,
                 float mag_thresh);

std::vector<cv::Mat> generate_image_pyramid(const cv::Mat& image, int levels) {
  std::vector<cv::Mat> pyramid;
  if (image.empty() || levels <= 0) {
    std::cerr << "Error: empty image or pyramid_levels <= 0" << std::endl;
    return pyramid;
  }
  pyramid.reserve(levels);
  cv::Mat current_level = image.clone();
  pyramid.push_back(current_level);

  for (int i = 1; i < levels; ++i) {
    cv::Mat next_level;
    cv::pyrDown(current_level, next_level);  // 使用 pyrDown 生成金字塔下层
    pyramid.push_back(next_level);
    current_level = next_level;
  }

  return pyramid;
}

std::vector<float> arange(float start, float stop, float step) {
  std::vector<float> result;

  if (step == 0.0f || !std::isfinite(start) || !std::isfinite(stop) ||
      !std::isfinite(step))
    return result;  // 防止死循环或无效输入

  float epsilon = std::abs(step) * 0.5f;

  // 预估容量：可选优化（提升性能）
  std::size_t estimated_size =
      static_cast<std::size_t>(std::ceil((stop - start) / step));
  result.reserve(estimated_size);

  if (step > 0) {
    for (float value = start; value < stop + epsilon; value += step) {
      result.push_back(value);
    }
  } else {
    for (float value = start; value > stop - epsilon; value += step) {
      result.push_back(value);
    }
  }

  return result;
}

void generateTransImage(const cv::Mat& image,
                        float angle,
                        float scale,
                        cv::Mat& rotated_image,
                        cv::Mat& rotated_mask,
                        const cv::Point2f& center) {
  int h = image.rows;
  int w = image.cols;

  // 创建全白的mask
  cv::Mat mask = cv::Mat::ones(h, w, CV_8UC1) * 255;

  // 使用默认中心（图像中心）
  cv::Point2f rotation_center = center;
  if (center.x < 0 || center.y < 0) {
    rotation_center = cv::Point2f(w / 2.0f, h / 2.0f);
  }

  // 获取旋转缩放矩阵（注意：角度为负）
  cv::Mat M = cv::getRotationMatrix2D(rotation_center, -angle, scale);

  // 计算新的尺寸
  double cos_val = std::abs(M.at<double>(0, 0));
  double sin_val = std::abs(M.at<double>(0, 1));
  int new_w = static_cast<int>(h * sin_val + w * cos_val + 0.5);
  int new_h = static_cast<int>(h * cos_val + w * sin_val + 0.5);

  // 调整旋转矩阵的平移部分
  M.at<double>(0, 2) += (new_w / 2.0) - rotation_center.x;
  M.at<double>(1, 2) += (new_h / 2.0) - rotation_center.y;

  // 应用仿射变换到图像和掩膜
  cv::warpAffine(image, rotated_image, M, cv::Size(new_w, new_h),
                 cv::INTER_LINEAR);
  cv::warpAffine(mask, rotated_mask, M, cv::Size(new_w, new_h),
                 cv::INTER_LINEAR);
}

void findPositionsGreaterThan(const cv::Mat& float_mat,
                              float threshold,
                              std::vector<cv::Point>& output_points) {
  output_points.clear();
  if (float_mat.empty() ||
      (float_mat.type() != CV_32F && float_mat.type() != CV_64F)) {
    throw std::invalid_argument("只支持 CV_32F 或 CV_64F 的浮点图像");
  }

  cv::Mat mask;
  cv::compare(float_mat, threshold, mask,
              cv::CMP_GT);  // 得到掩码图，float_mat > threshold 的位置为 255
  cv::findNonZero(mask, output_points);  // 提取非零点（即满足条件的位置）
}

void maxMergeWithIndex(const std::vector<cv::Mat>& images,
                       cv::Mat& max_result,
                       cv::Mat& index_map) {
  const int rows = images[0].rows;
  const int cols = images[0].cols;
  max_result = images[0].clone();
  index_map = cv::Mat::zeros(rows, cols, CV_16UC1);  // 支持多于255张图
  for (size_t idx = 1; idx < images.size(); ++idx) {
    const cv::Mat& img = images[idx];
    for (int r = 0; r < rows; ++r) {
      const float* p_img = img.ptr<float>(r);
      float* p_max = max_result.ptr<float>(r);
      uint16_t* p_idx = index_map.ptr<uint16_t>(r);

      for (int c = 0; c < cols; ++c) {
        if (p_img[c] >= p_max[c]) {
          p_max[c] = p_img[c];
          p_idx[c] = static_cast<uint16_t>(idx);
        }
      }
    }
  }
}

void findMaxima(const cv::Mat& image,
                std::vector<cv::Point>& max_points,
                float threshold) {
  max_points.clear();
  const int rows = image.rows;
  const int cols = image.cols;
  for (int row = 1; row < rows - 1; ++row) {
    const float* prev = image.ptr<float>(row - 1);
    const float* curr = image.ptr<float>(row);
    const float* next = image.ptr<float>(row + 1);

    for (int col = 1; col < cols - 1; ++col) {
      float center_val = curr[col];
      if (center_val < threshold)
        continue;
      // 直接访问邻居，避免使用 image.at()
      if (center_val >= prev[col - 1] && center_val >= prev[col] &&
          center_val >= prev[col + 1] && center_val >= curr[col - 1] &&
          center_val >= curr[col + 1] && center_val >= next[col - 1] &&
          center_val >= next[col] && center_val >= next[col + 1]) {
        max_points.emplace_back(col, row);
      }
    }
  }
}
std::vector<cv::Mat> readImagesInFolder(const std::string& folderPath) {
  std::vector<cv::Mat> images;

  // Iterate through all files in the folder
  for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
    // Check if current entry is a file
    if (entry.is_regular_file()) {
      // Read the image in grayscale
      cv::Mat image = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE);

      // Check if the image was read successfully
      if (!image.empty()) {
        images.push_back(image);
      } else {
        std::cerr << "Failed to read image: " << entry.path().filename()
                  << std::endl;
      }
    }
  }

  return images;
}

void QuantifyEdge(cv::Mat image,
                  cv::Mat& angle,
                  cv::Mat& quantized_angle,
                  cv::Mat& mag,
                  float mag_thresh,
                  bool calc_180) {
  cv::Mat dx, dy;
  // 假设 dx 和 dy 是通过 Sobel、filter2D 等方式得到的
  // 例如：
  cv::Sobel(image, dx, CV_32F, 1, 0, 3);
  cv::Sobel(image, dy, CV_32F, 0, 1, 3);

  // 一步计算幅值和角度（angleInDegrees = true 表示角度制）
  cv::cartToPolar(dx, dy, mag, angle, true);
  cv::Mat mask = angle >= 180;  // 正确方式，生成掩膜
  cv::subtract(angle, 180.0f, angle, mask);
  if (calc_180)
    Quantify180(angle, quantized_angle, mag, mag_thresh);
  else
    Quantify16(angle, quantized_angle, mag, mag_thresh);
}

void Quantify16(cv::Mat& angle,
                cv::Mat& quantized_angle,
                cv::Mat& mag,
                float mag_thresh) {
  quantized_angle = cv::Mat::zeros(angle.size(), CV_8U);
  // #pragma omp parallel for
  for (int r = 0; r < angle.rows; ++r) {
    unsigned char* quantized_angle_ptr = quantized_angle.ptr<unsigned char>(r);
    float* angle_ptr = angle.ptr<float>(r);
    float* mag_ptr = mag.ptr<float>(r);
    for (int c = 0; c < angle.cols; ++c) {
      if (mag_ptr[c] >= mag_thresh)
        quantized_angle_ptr[c] = (int)(angle_ptr[c] * 16.0f / 180.0f);
      else
        quantized_angle_ptr[c] = 255;
    }
  }
}

void Quantify180(cv::Mat angle,
                 cv::Mat& quantized_angle,
                 cv::Mat mag,
                 float mag_thresh) {
  quantized_angle = cv::Mat::zeros(angle.size(), CV_8U);
  // #pragma omp parallel for
  for (int r = 0; r < angle.rows; ++r) {
    unsigned char* quantized_angle_ptr = quantized_angle.ptr<unsigned char>(r);
    float* angle_ptr = angle.ptr<float>(r);
    float* mag_ptr = mag.ptr<float>(r);
    for (int c = 0; c < angle.cols; ++c) {
      if (mag_ptr[c] >= (mag_thresh * mag_thresh))
        quantized_angle_ptr[c] = (int)round(angle_ptr[c]) % 180;
      else
        quantized_angle_ptr[c] = 255;
    }
  }
}

std::vector<int> farthest_point_sampling(const std::vector<cv::Point>& points,
                                         int num_samples) {
  int N = static_cast<int>(points.size());
  if (N == 0 || num_samples <= 0)
    return {};

  std::vector<int> selected_indices;
  selected_indices.reserve(num_samples);

  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  int first_idx = std::rand() % N;
  selected_indices.push_back(first_idx);

  std::vector<float> min_dists(N, std::numeric_limits<float>::max());

  for (int i = 1; i < num_samples; ++i) {
    const cv::Point& last_point = points[selected_indices.back()];

    for (int j = 0; j < N; ++j) {
      float dx = static_cast<float>(last_point.x - points[j].x);
      float dy = static_cast<float>(last_point.y - points[j].y);
      float dist = std::sqrt(dx * dx + dy * dy);
      if (dist < min_dists[j])
        min_dists[j] = dist;
    }

    float max_dist = -1.0f;
    int farthest_idx = 0;
    for (int j = 0; j < N; ++j) {
      if (min_dists[j] > max_dist) {
        max_dist = min_dists[j];
        farthest_idx = j;
      }
    }

    selected_indices.push_back(farthest_idx);
  }

  return selected_indices;
}
}  // namespace utils
}  // namespace

namespace shape_match {
void TemplData::auto_calculate_pyramid_levels() {
  if (pyramid_levels > 0) {
    return;
  }
  int m = std::min(image.cols, image.rows);
  int count = 1;
  while (m > 32) {
    m /= 2;
    count++;
  }
  pyramid_levels = count;
}
void TemplData::auto_calculate_thresholds(std::vector<cv::Mat>& imgs) {
  for (int i = 0; i < pyramid_levels; i++) {
    cv::Mat& image = imgs[i];
    // 1. Sobel 计算梯度幅值
    cv::Mat grad_x, grad_y;
    Sobel(image, grad_x, CV_32F, 1, 0, 3);
    Sobel(image, grad_y, CV_32F, 0, 1, 3);
    cv::Mat magnitude;
    magnitude = grad_x.mul(grad_x) + grad_y.mul(grad_y);
    cv::sqrt(magnitude, magnitude);  // CV_32F 类型

    // 2. 归一化到 0~255 用于 Otsu（注意保留最大最小值）
    double minVal, maxVal;
    cv::minMaxLoc(magnitude, &minVal, &maxVal);  // 记录原始范围

    cv::Mat mag_norm_8u;
    magnitude.convertTo(mag_norm_8u, CV_8U, 255.0 / maxVal);  // [0, 255]

    // 3. 用 Otsu 计算阈值（在 0~255 空间）
    cv::Mat dummy;
    double otsu_thresh_8u = threshold(mag_norm_8u, dummy, 0, 255,
                                      cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 4. 反归一化为原始幅值域的阈值
    double real_otsu_thresh = otsu_thresh_8u / 255.0 * maxVal;  // √(Gx² + Gy²)
    pyramid_thresholds.push_back(static_cast<float>(real_otsu_thresh));
  }
}
void TemplData::auto_calculate_angle_step() {
  int m = std::max(image.cols, image.rows);
  pyramid_angle_steps.reserve(pyramid_levels);
  for (int i = 0; i < pyramid_levels; i++) {
    pyramid_angle_steps.push_back(std::asin(1.0f / m) * 180.0f / CV_PI);
    m /= 2;
  }
  if (step_angle > 0)
    pyramid_angle_steps[0] = step_angle;
  else if (step_angle <= 0)
    step_angle = pyramid_angle_steps[0];
}
void TemplData::save(const std::string& filename) const {
  std::ofstream os(filename, std::ios::binary);
  if (!os)
    throw std::runtime_error("无法打开文件用于保存");

  // 匿名 lambda 函数写图像
  auto write_mat = [&os](const cv::Mat& mat) {
    int rows = mat.rows, cols = mat.cols, type = mat.type();
    os.write(reinterpret_cast<const char*>(&rows), sizeof(int));
    os.write(reinterpret_cast<const char*>(&cols), sizeof(int));
    os.write(reinterpret_cast<const char*>(&type), sizeof(int));
    os.write(reinterpret_cast<const char*>(mat.data),
             mat.total() * mat.elemSize());
  };
  write_mat(image);

  // 保存 float 参数
  os.write(reinterpret_cast<const char*>(&start_angle), sizeof(float));
  os.write(reinterpret_cast<const char*>(&stop_angle), sizeof(float));
  os.write(reinterpret_cast<const char*>(&step_angle), sizeof(float));
  os.write(reinterpret_cast<const char*>(&start_scale), sizeof(float));
  os.write(reinterpret_cast<const char*>(&stop_scale), sizeof(float));
  os.write(reinterpret_cast<const char*>(&step_scale), sizeof(float));

  // 保存 pyramid 数据
  auto write_vec = [&os](const std::vector<float>& vec) {
    size_t size = vec.size();
    os.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    os.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(float));
  };
  write_vec(pyramid_angle_steps);
  write_vec(pyramid_scale_steps);
  write_vec(pyramid_thresholds);

  os.write(reinterpret_cast<const char*>(&pyramid_levels), sizeof(int));
  os.write(reinterpret_cast<const char*>(&template_original_center),
           sizeof(cv::Point2f));
  os.write(reinterpret_cast<const char*>(&rect_roi), sizeof(cv::Rect));

  // 保存模板列表
  size_t outer_size = templates_list.size();
  os.write(reinterpret_cast<const char*>(&outer_size), sizeof(size_t));
  for (const auto& templs : templates_list) {
    size_t inner_size = templs.size();
    os.write(reinterpret_cast<const char*>(&inner_size), sizeof(size_t));
    for (const auto& templ : templs) {
      os.write(reinterpret_cast<const char*>(&templ.id), sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.pyramid_level),
               sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.is_valid), sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.x), sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.y), sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.w), sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.h), sizeof(int));
      os.write(reinterpret_cast<const char*>(&templ.angle), sizeof(float));
      os.write(reinterpret_cast<const char*>(&templ.scale), sizeof(float));

      size_t feat_size = templ.features.size();
      os.write(reinterpret_cast<const char*>(&feat_size), sizeof(size_t));
      os.write(reinterpret_cast<const char*>(templ.features.data()),
               feat_size * sizeof(Feature));
    }
  }
}
void TemplData::load(const std::string& filename) {
  std::ifstream is(filename, std::ios::binary);
  if (!is)
    throw std::runtime_error("无法打开文件用于加载");

  // 匿名 lambda 函数读图像
  auto read_mat = [&is]() {
    int rows, cols, type;
    is.read(reinterpret_cast<char*>(&rows), sizeof(int));
    is.read(reinterpret_cast<char*>(&cols), sizeof(int));
    is.read(reinterpret_cast<char*>(&type), sizeof(int));
    cv::Mat mat(rows, cols, type);
    is.read(reinterpret_cast<char*>(mat.data), mat.total() * mat.elemSize());
    return mat;
  };
  image = read_mat();

  is.read(reinterpret_cast<char*>(&start_angle), sizeof(float));
  is.read(reinterpret_cast<char*>(&stop_angle), sizeof(float));
  is.read(reinterpret_cast<char*>(&step_angle), sizeof(float));
  is.read(reinterpret_cast<char*>(&start_scale), sizeof(float));
  is.read(reinterpret_cast<char*>(&stop_scale), sizeof(float));
  is.read(reinterpret_cast<char*>(&step_scale), sizeof(float));

  auto read_vec = [&is](std::vector<float>& vec) {
    size_t size;
    is.read(reinterpret_cast<char*>(&size), sizeof(size_t));
    vec.resize(size);
    is.read(reinterpret_cast<char*>(vec.data()), size * sizeof(float));
  };
  read_vec(pyramid_angle_steps);
  read_vec(pyramid_scale_steps);
  read_vec(pyramid_thresholds);

  is.read(reinterpret_cast<char*>(&pyramid_levels), sizeof(int));
  is.read(reinterpret_cast<char*>(&template_original_center),
          sizeof(cv::Point2f));
  is.read(reinterpret_cast<char*>(&rect_roi), sizeof(cv::Rect));

  // 加载模板列表
  size_t outer_size;
  is.read(reinterpret_cast<char*>(&outer_size), sizeof(size_t));
  templates_list.resize(outer_size);
  for (auto& templs : templates_list) {
    size_t inner_size;
    is.read(reinterpret_cast<char*>(&inner_size), sizeof(size_t));
    templs.resize(inner_size);
    for (auto& templ : templs) {
      is.read(reinterpret_cast<char*>(&templ.id), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.pyramid_level), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.is_valid), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.x), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.y), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.w), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.h), sizeof(int));
      is.read(reinterpret_cast<char*>(&templ.angle), sizeof(float));
      is.read(reinterpret_cast<char*>(&templ.scale), sizeof(float));

      size_t feat_size;
      is.read(reinterpret_cast<char*>(&feat_size), sizeof(size_t));
      templ.features.resize(feat_size);
      is.read(reinterpret_cast<char*>(templ.features.data()),
              feat_size * sizeof(Feature));
    }
  }
}

void TemplData::set_original_center(float x, float y) {
  template_original_center = cv::Point2f(x, y);
  rect_roi =
      cv::Rect(x - image.cols / 2, y - image.rows / 2, image.cols, image.rows);
}

Template ExtractTemplate(cv::Mat angle,
                         cv::Mat quantized_angle,
                         cv::Mat mag,
                         int pyr_level,
                         float weak_thresh,
                         float strong_thresh,
                         int num_features,
                         cv::Mat mask) {
  cv::Mat local_angle = cv::Mat(angle.size(), angle.type());
  for (int r = 0; r < angle.rows; ++r) {
    float* angle_ptr = angle.ptr<float>(r);
    float* local_angle_ptr = local_angle.ptr<float>(r);
    for (int c = 0; c < angle.cols; ++c) {
      float dir = angle_ptr[c];
      if (dir < 22.5 || dir > 157.5)
        local_angle_ptr[c] = 0.f;
      else if (dir > 22.5 && dir < 67.5)
        local_angle_ptr[c] = 45.f;
      else if (dir > 67.5 && dir < 112.5)
        local_angle_ptr[c] = 90.f;
      else if (dir > 112.5 && dir < 157.5)
        local_angle_ptr[c] = 135.f;
      else
        local_angle_ptr[c] = 0.f;
    }
  }

  std::vector<Candidate> candidates;
  candidates.clear();
  bool no_mask = mask.empty();
  float weak_sq = weak_thresh;
  float strong_sq = strong_thresh;
  float pre_grad, lst_grad;
  for (int r = 1; r < mag.rows - 1; ++r) {
    const unsigned char* mask_ptr = no_mask ? NULL : mask.ptr<unsigned char>(r);
    const float* pre_ptr = mag.ptr<float>(r - 1);
    const float* cur_ptr = mag.ptr<float>(r);
    const float* lst_ptr = mag.ptr<float>(r + 1);
    float* local_angle_ptr = local_angle.ptr<float>(r);

    for (int c = 1; c < mag.cols - 1; ++c) {
      if (no_mask || mask_ptr[c]) {
        switch ((int)local_angle_ptr[c]) {
          case 0:
            pre_grad = cur_ptr[c - 1];
            lst_grad = cur_ptr[c + 1];
            break;
          case 45:
            pre_grad = pre_ptr[c - 1];
            lst_grad = lst_ptr[c + 1];
            break;
          case 90:
            pre_grad = pre_ptr[c];
            lst_grad = lst_ptr[c];
            break;
          case 135:
            pre_grad = pre_ptr[c + 1];
            lst_grad = lst_ptr[c - 1];
            break;
        }
        if ((cur_ptr[c] >= pre_grad) && (cur_ptr[c] >= lst_grad)) {
          float score = cur_ptr[c];
          bool validity = false;
          if (score >= weak_sq) {
            if (score >= strong_sq) {
              validity = true;
            } else {
              if (((pre_ptr[c - 1]) >= strong_sq) ||
                  ((pre_ptr[c]) >= strong_sq) ||
                  ((pre_ptr[c + 1]) >= strong_sq) ||
                  ((cur_ptr[c - 1]) >= strong_sq) ||
                  ((cur_ptr[c + 1]) >= strong_sq) ||
                  ((lst_ptr[c - 1]) >= strong_sq) ||
                  ((lst_ptr[c]) >= strong_sq) ||
                  ((lst_ptr[c + 1]) >= strong_sq)) {
                validity = true;
              }
            }
          }
          if (validity == true &&
              quantized_angle.at<unsigned char>(r, c) != 255) {
            Candidate cd;
            cd.score = score;
            cd.feature.x = c;
            cd.feature.y = r;
            cd.feature.lbl = quantized_angle.at<unsigned char>(r, c);
            cd.feature.angle = angle.at<float>(r, c);
            candidates.push_back(cd);
          }
        }
      }
    }
  }

  Template templ;
  templ.angle = 0;
  templ.scale = 1;
  templ.x = 0;
  templ.y = 0;
  templ.w = angle.cols;
  templ.h = angle.rows;
  templ.pyramid_level = pyr_level;
  templ.is_valid = 0;
  templ.features.clear();

  if (candidates.size() >= num_features && num_features > 0) {
    /*std::stable_sort(candidates.begin(), candidates.end());*/
    /*float distance = static_cast<float>(candidates.size() / num_features +
     * 1);*/
    std::vector<int> index = SelectScatteredFeatures(candidates, num_features);
    for (int i = 0; i < index.size(); ++i) {
      templ.features.push_back(candidates[index[i]].feature);
    }
    /*for (int c = 0; c < num_features; c++) {
      templ.features.push_back(candidates[c].feature);
    }*/
  } else {
    for (int c = 0; c < candidates.size(); c++) {
      templ.features.push_back(candidates[c].feature);
    }
  }

  if (templ.features.size() < 10) {
    std::cout << "Warning: template has less than 10 features." << std::endl;
  } else {
    templ.is_valid = 0;
  }

  return templ;
}

void CropTemplate(Template& templ) {
  int min_x = std::numeric_limits<int>::max();
  int min_y = std::numeric_limits<int>::max();
  int max_x = std::numeric_limits<int>::min();
  int max_y = std::numeric_limits<int>::min();

  // First pass: find min/max feature x,y
  for (int i = 0; i < (int)templ.features.size(); ++i) {
    int x = templ.features[i].x;
    int y = templ.features[i].y;
    min_x = std::min(min_x, x);
    min_y = std::min(min_y, y);
    max_x = std::max(max_x, x);
    max_y = std::max(max_y, y);
  }

  ///// @todo Why require even min_x, min_y?
  // if (min_x % 2 == 1)
  //   --min_x;
  // if (min_y % 2 == 1)
  //   --min_y;

  // Second pass: set width/height and shift all feature positions
  templ.w = (max_x - min_x) + 1;
  templ.h = (max_y - min_y) + 1;
  templ.x = min_x;
  templ.y = min_y;

  for (int i = 0; i < (int)templ.features.size(); ++i) {
    templ.features[i].x -= templ.x;
    templ.features[i].y -= templ.y;
  }
}
std::vector<int> SelectScatteredFeatures(std::vector<Candidate>& candidates,
                                         int num_features) {
  std::vector<cv::Point> points;
  for (int i = 0; i < (int)candidates.size(); ++i) {
    points.emplace_back(candidates[i].feature.x, candidates[i].feature.y);
  }
  return utils::farthest_point_sampling(points, num_features);
}

void AddTemplate_Transform(Template& templ,
                           float angle,
                           float scale,
                           bool calc_180) {
  float cx = templ.x + templ.w / 2.0f;
  float cy = templ.y + templ.h / 2.0f;
  cv::Point2f center(cx, cy);
  // 原始特征点 -> 图像坐标系中点
  std::vector<cv::Point2f> points;
  for (const auto& f : templ.features) {
    points.emplace_back(f.x, f.y);
  }
  // 获取旋转+缩放矩阵
  cv::Mat rot_mat = cv::getRotationMatrix2D(center, -angle, scale);
  // 扩展点为齐次坐标并变换
  std::vector<cv::Point2f> rotated_points;
  cv::transform(points, rotated_points, rot_mat);
  // 旋转+缩放模板
  for (size_t i = 0; i < templ.features.size(); ++i) {
    templ.features[i].x = int(rotated_points[i].x + 0.5);
    templ.features[i].y = int(rotated_points[i].y + 0.5);

    float new_theta = templ.features[i].angle + angle;
    while (new_theta >= 180.0f)
      new_theta -= 180.0f;
    while (new_theta < 0.0f)
      new_theta += 180.0f;
    templ.features[i].angle = new_theta;
    templ.features[i].lbl = calc_180
                                ? static_cast<int>(new_theta)
                                : static_cast<int>(new_theta * 16.0f / 180.0f);
  }
  CropTemplate(templ);
}
TemplData create_shape_model(const cv::Mat& img,
                             float start_angle,
                             float stop_angle,
                             float step_angle,
                             float start_scale,
                             float stop_scale,
                             float step_scale,
                             int pyramid_levels,
                             int num_features,
                             cv::Mat mask) {
  // 检查输入参数的有效性
  if (img.channels() != 1) {
    throw std::invalid_argument("模板图像必须是单通道图像");
  }
  if (start_angle > stop_angle) {
    throw std::invalid_argument("起始角度必须小于等于结束角度");
  }
  if (start_scale > stop_scale) {
    throw std::invalid_argument("起始尺度必须小于等于结束尺度");
  }
  TemplData templdata;
  templdata.image = img.clone();
  templdata.template_original_center =
      cv::Point2f(img.cols / 2.0f, img.rows / 2.0f);
  templdata.rect_roi = cv::Rect(0, 0, img.cols, img.rows);

  templdata.start_angle = start_angle;
  templdata.stop_angle = stop_angle;
  templdata.step_angle = step_angle;
  templdata.start_scale = start_scale;
  templdata.stop_scale = stop_scale;
  templdata.step_scale = step_scale;
  templdata.pyramid_levels = pyramid_levels;

  // 自动计算金字塔层数
  templdata.auto_calculate_pyramid_levels();
  templdata.pyramid_angle_steps.resize(templdata.pyramid_levels);
  // 自动计算尺度步长
  //@todo templdata.auto_calculate_scale_steps();

  // 计算金字塔图像
  std::vector<cv::Mat> pyramid_images =
      utils::generate_image_pyramid(img, templdata.pyramid_levels);

  // 自动计算每层金字塔图像的合适阈值
  templdata.auto_calculate_thresholds(pyramid_images);

  // 计算掩膜金字塔图像
  if (mask.empty()) {
    mask = cv::Mat(img.size(), CV_8UC1, cv::Scalar(255));
  }
  std::vector<cv::Mat> mask_pyramid_images =
      utils::generate_image_pyramid(mask, templdata.pyramid_levels);
  // 对每张金字塔图像进行处理
  for (int i = 0; i < templdata.pyramid_levels; i++) {
    cv::Mat& pyramid_image = pyramid_images[i];
    cv::Mat& mask_image = mask_pyramid_images[i];
    cv::Mat mag, angle, quantized_angle16;
    // 计算图像角度，梯度，量化后的角度
    utils::QuantifyEdge(pyramid_image, angle, quantized_angle16, mag,
                        templdata.pyramid_thresholds[i] / 4, 0);

    Template templ = ExtractTemplate(
        angle, quantized_angle16, mag, i, templdata.pyramid_thresholds[i] / 4,
        templdata.pyramid_thresholds[i], num_features / (1 << i), mask_image);
    // cv::Mat draw_img;
    // cv::cvtColor(pyramid_image, draw_img, cv::COLOR_GRAY2BGR);
    // for (auto& f : templ.features) {
    //   cv::circle(draw_img, cv::Point(f.x, f.y), 1, cv::Scalar(0, 0, 255),
    //   -1);
    // }
    // cv::imshow("ff", draw_img);
    // cv::waitKey(0);
    // 计算角度步长
    int m = std::max(templ.h, templ.w);
    templdata.pyramid_angle_steps[i] = (std::asin(1.0f / m) * 180.0f / CV_PI);
    //   计算当前金字塔旋转缩放所有模板
    std::vector<Template> templates;
    // 生成角度和缩放值的范围，确保包含结束值
    std::vector<float> angle_values = utils::arange(
        start_angle, stop_angle, templdata.pyramid_angle_steps[i]);
    std::vector<float> scale_values =
        utils::arange(start_scale, stop_scale, step_scale);
    templates.resize(angle_values.size() * scale_values.size());
    for (int j = 0; j < angle_values.size(); j++) {
      for (int k = 0; k < scale_values.size(); k++) {
        Template new_templ = templ;
        new_templ.id = k + j * scale_values.size();
        new_templ.scale = scale_values[k];
        new_templ.angle = angle_values[j];
        AddTemplate_Transform(new_templ, angle_values[j], scale_values[k], 0);
        templates[new_templ.id] = new_templ;
      }
    }
    templdata.templates_list.push_back(templates);
  }
  return templdata;
};
std::vector<MatchResult> find_shape_model(const cv::Mat& img,
                                          TemplData templ_data,
                                          cv::Rect rect,
                                          float score_threshold,
                                          float overlap_threshold,
                                          int max_results,
                                          float mag_thresh,
                                          int pyramid_level) {
  if (img.channels() != 1) {
    throw std::invalid_argument("输入图像必须是单通道图像");
  }
  std::vector<MatchResult> results;
  int current_pyramid_levels;
  if (pyramid_level > 0 && pyramid_level <= templ_data.pyramid_levels)
    current_pyramid_levels = pyramid_level;
  else
    current_pyramid_levels = templ_data.pyramid_levels;
  //   计算金字塔图像
  std::vector<cv::Mat> pyramid_images =
      utils::generate_image_pyramid(img, current_pyramid_levels);
  // 最顶层模板进行粗匹配
  std::vector<Template>& templates =
      templ_data.templates_list[current_pyramid_levels - 1];
  std::vector<cv::Mat> trans_results;
  trans_results.resize(templates.size());
  cv::Mat angle;
  cv::Mat quantized_angle;
  cv::Mat mag;
  utils::QuantifyEdge(pyramid_images[current_pyramid_levels - 1], angle,
                      quantized_angle, mag, mag_thresh / 5, 0);
#pragma omp parallel for
  for (int t = 0; t < templates.size(); t++) {
    trans_results[t] = Matching_16_simd(quantized_angle, templates[t]);
  }
  cv::Mat match_result;
  cv::Mat index_result;
  utils::maxMergeWithIndex(trans_results, match_result, index_result);
  std::vector<cv::Point> points;
  utils::findMaxima(match_result, points, score_threshold * 0.6);
  results.resize(points.size());
  for (int i = 0; i < points.size(); i++) {
    cv::Point& point = points[i];
    MatchResult& match = results[i];
    int index = index_result.at<uint16_t>(point);
    // match.template_id = templates[index].template_id;
    match.similarity = match_result.at<float>(point);
    match.x = point.x + rect.x;
    match.y = point.y + rect.y;
    match.w = templates[index].w;
    match.h = templates[index].h;
    match.scale = templates[index].scale;
    match.angle = templates[index].angle;
  }
  if (current_pyramid_levels > 1) {
    // 递归处理金字塔的其他层
    for (int i = current_pyramid_levels - 2; i >= 0; i--) {
      cv::Mat angle;
      cv::Mat quantized_angle;
      cv::Mat mag;
      utils::QuantifyEdge(pyramid_images[i], angle, quantized_angle, mag,
                          mag_thresh * pow(0.7, i), 0);
      std::vector<MatchResult> new_results;
      new_results.resize(results.size());
#pragma omp parallel for
      for (int r = 0; r < results.size(); r++) {
        MatchResult& result = results[r];
        int roi_size = 5;
        cv::Rect rect_roi(result.x * 2 - roi_size, result.y * 2 - roi_size,
                          roi_size * 2, roi_size * 2);
        float max_score = 0;
        for (int j = 0; j < templ_data.templates_list[i].size(); j++) {
          float angle_thre = templ_data.pyramid_angle_steps[i + 1];
          Template& tmpl = templ_data.templates_list[i][j];
          if (tmpl.angle < result.angle - 3 * angle_thre ||
              tmpl.angle > result.angle + 3 * angle_thre ||
              tmpl.scale < result.scale * 0.9f ||
              tmpl.scale > result.scale * 1.1f)
            continue;
          rect_roi.width = tmpl.w + roi_size * 2;
          rect_roi.height = tmpl.h + roi_size * 2;
          cv::Rect safe_rect = rect_roi & cv::Rect(0, 0, pyramid_images[i].cols,
                                                   pyramid_images[i].rows);
          if (safe_rect.width <= tmpl.w || safe_rect.height <= tmpl.h)
            continue;
          cv::Mat quantized_angle_roi = quantized_angle(safe_rect).clone();
          cv::Mat shape_result = Matching_16(quantized_angle_roi, tmpl);
          double minVal, maxVal;
          cv::Point minLoc, maxLoc;
          // 查找最大值和最大值的位置
          cv::minMaxLoc(shape_result, &minVal, &maxVal, &minLoc, &maxLoc);
          if (maxVal > max_score) {
            max_score = maxVal;
            MatchResult match;
            match.template_id = tmpl.id;
            match.similarity = max_score;
            match.x = maxLoc.x + rect.x + safe_rect.x;
            match.y = maxLoc.y + rect.y + safe_rect.y;
            match.w = tmpl.w;
            match.h = tmpl.h;
            match.scale = tmpl.scale;
            match.angle = tmpl.angle;
            new_results[r] = match;
          }
        }
      }
      //// 筛选结果,进行非极大值抑制
      if (i == 0)
        results = doNMSMatches(new_results, score_threshold, overlap_threshold);
      else
        results = doNMSMatches(new_results, score_threshold * pow(0.8, i), 0.8);

      // cv::Mat output;
      // cv::cvtColor(pyramid_images[i], output, cv::COLOR_GRAY2BGR);
      // draw_shape_results(output, results);
      // cv::imshow("匹配结果", output);
      // cv::waitKey(0);
      // std::cout << "第" << i + 1 << "层匹配结果: " << results.size()
      //           << std::endl;
    }
  }

  results = doNMSMatches(results, score_threshold, overlap_threshold);
  results = GetTopKMatches(results, max_results);
  // 计算模板到目标位置的变换矩阵
  // #pragma omp parallel for
  for (int i = 0; i < results.size(); ++i) {
    auto& match = results[i];
    cv::Matx23d mat = cv::getRotationMatrix2D(
        templ_data.template_original_center, -match.angle, match.scale);
    mat(0, 2) +=
        match.x + match.w / 2.0f - templ_data.template_original_center.x;
    mat(1, 2) +=
        match.y + match.h / 2.0f - templ_data.template_original_center.y;
    match.transform_matrix = {
        static_cast<float>(mat(0, 0)), static_cast<float>(mat(0, 1)),
        static_cast<float>(mat(0, 2)), static_cast<float>(mat(1, 0)),
        static_cast<float>(mat(1, 1)), static_cast<float>(mat(1, 2))};
    match.features = templ_data.templates_list[0][match.template_id].features;
  }

  return results;
}
TemplData read_shape_model(const std::string& filename) {
  FILE* fp = fopen(filename.c_str(), "rb");
  if (!fp) {
    // std::cerr << "Failed to open file: " << filename << std::endl;
    throw std::runtime_error("Cannot open file for reading.");
  }
  TemplData templ_data;
  templ_data.load(filename);
  return templ_data;
}
void draw_shape_results(const cv::Mat& image,
                        const std::vector<MatchResult>& results,
                        const cv::Scalar& color,
                        int thickness) {
  if (image.empty()) {
    throw std::invalid_argument("输入图像不能为空");
  }
  if (results.empty()) {
    std::cout << "no results to draw!" << std::endl;
    return;
  }
  for (const auto& result : results) {
    // 确保绘制区域合法
    cv::Rect rect(result.x, result.y, result.w, result.h);
    cv::Rect img_rect(0, 0, image.cols, image.rows);
    cv::Rect clipped = rect & img_rect;
    if (clipped.empty())
      continue;
    // 画边框
    /*   cv::rectangle(image, clipped, color, thickness);*/

    // 拼接显示信息：相似度、角度、尺度
    std::ostringstream oss;
    oss << "Sim: " << std::fixed << std::setprecision(2) << result.similarity
        << " Ang: " << std::setprecision(1) << result.angle
        << " S: " << std::setprecision(2) << result.scale;

    // 设置文本显示位置（左上角稍微偏移）
    int text_x = std::max(result.x, 0);
    int text_y = std::max(result.y - 5, 10);

    cv::putText(image, oss.str(), cv::Point(text_x, text_y),
                cv::FONT_HERSHEY_PLAIN, 1.0, color, 1, cv::LINE_AA);

    if (result.features.empty()) {
      continue;  // 如果没有特征点，跳过绘制
    } else {
      for (const auto& feat : result.features) {
        int fx = static_cast<int>(feat.x + result.x);
        int fy = static_cast<int>(feat.y + result.y);
        if (fx >= 0 && fx < image.cols && fy >= 0 && fy < image.rows) {
          cv::circle(image, cv::Point(fx, fy), 1, color, -1);
          /*cv::putText(image, std::to_string(feat.lbl), cv::Point(fx, fy),
                      cv::FONT_HERSHEY_PLAIN, 1.0, color, 1, cv::LINE_AA);*/
        }
      }
      // 绘制特征点（加边界检查）
    }
  }
}
std::vector<MatchResult> doNMSMatches(const std::vector<MatchResult>& matches,
                                      float score_thresh,
                                      float overlap_thresh) {
  std::vector<cv::Rect> boxes;
  std::vector<float> scores;

  for (const auto& m : matches) {
    boxes.emplace_back(cv::Rect(m.x, m.y, m.w, m.h));
    scores.emplace_back(m.similarity);
  }

  std::vector<int> indices;
  cv::dnn::NMSBoxes(boxes, scores, score_thresh, overlap_thresh, indices);

  std::vector<MatchResult> final_matches;
  for (int idx : indices) {
    final_matches.push_back(matches[idx]);
  }
  return final_matches;
}
// 选取前 k 个匹配结果
std::vector<MatchResult> GetTopKMatches(std::vector<MatchResult>& matches,
                                        int top_k_) {
  if (top_k_ <= 0 || matches.empty())
    return {};

  std::sort(matches.begin(), matches.end());  // 自动按你定义的 operator< 排序

  int k = std::min(static_cast<int>(matches.size()), top_k_);
  return std::vector<MatchResult>(matches.begin(), matches.begin() + k);
}

cv::Mat Matching_16(cv::Mat& quantized_angle,
                    const Template& templ) {  // 注意传引用
  const int valid_h = quantized_angle.rows - templ.h + 1;
  const int valid_w = quantized_angle.cols - templ.w + 1;
  const int fsz = static_cast<int>(templ.features.size());

  cv::Mat score_map(quantized_angle.rows, quantized_angle.cols, CV_32FC1,
                    cv::Scalar(0));

  for (int i = 0; i < valid_h; ++i) {
    float* score_row_ptr = score_map.ptr<float>(i);
    for (int j = 0; j < valid_w; ++j) {
      float partial_sum = 0.0f;
      for (int f = 0; f < fsz; ++f) {
        const Feature& feat = templ.features[f];
        int y = i + feat.y;
        int x = j + feat.x;

        uchar label = quantized_angle.at<uchar>(y, x);
        if (label == 255)
          continue;
        partial_sum += (score_table_16d + feat.lbl * 16)[label];
      }
      score_row_ptr[j] = partial_sum / 255 / fsz;
    }
  }
  return score_map;
}
inline void accumulate_u8_to_f32_avx2(__m128i u8, float* dst) {
  // 拆出低8个字节 -> 扩展到 32-bit int -> 转 float
  __m256i u8_lo_32 = _mm256_cvtepu8_epi32(u8);  // 低8字节
  __m256 f_lo = _mm256_cvtepi32_ps(u8_lo_32);   // 转 float

  // 拆出高8个字节（通过移位）
  __m128i u8_hi = _mm_srli_si128(u8, 8);           // 高8字节
  __m256i u8_hi_32 = _mm256_cvtepu8_epi32(u8_hi);  // 扩展到 32-bit int
  __m256 f_hi = _mm256_cvtepi32_ps(u8_hi_32);      // 转 float

  // 加载目标值，累加并存回
  __m256 dst_lo = _mm256_loadu_ps(dst);      // dst[0..7]
  __m256 dst_hi = _mm256_loadu_ps(dst + 8);  // dst[8..15]

  dst_lo = _mm256_add_ps(dst_lo, f_lo);
  dst_hi = _mm256_add_ps(dst_hi, f_hi);

  _mm256_storeu_ps(dst, dst_lo);
  _mm256_storeu_ps(dst + 8, dst_hi);
}
cv::Mat Matching_16_simd(cv::Mat& quantized_angle,
                         const Template templ  // 注意传引用
) {
  const int valid_h = quantized_angle.rows - templ.h + 1;
  const int valid_w = quantized_angle.cols - templ.w + 1;
  const int fsz = static_cast<int>(templ.features.size());

  cv::Mat score_map(quantized_angle.rows, quantized_angle.cols, CV_32FC1,
                    cv::Scalar(0));
  int block_num = valid_w / 16;

  for (int z = 0; z < templ.features.size(); z++) {
    int lbl = templ.features[z].lbl;
    __m128i tangle = _mm_load_si128(
        reinterpret_cast<const __m128i*>(score_table_16d + lbl * 16));
    int x0 = templ.features[z].x;
    int y0 = templ.features[z].y;
    for (int i = 0; i < valid_h; ++i) {
      float* score_row_ptr = score_map.ptr<float>(i);
      uchar* angle_row_ptr = quantized_angle.ptr<uchar>(i + y0);
      for (int j = 0; j < block_num * 16; j += 16) {
        uchar* angle_ptr = angle_row_ptr + j + x0;
        float* score_ptr = score_row_ptr + j;
        __m128i qangle = _mm_loadu_si128((__m128i*)angle_ptr);
        // 计算分数
        __m128i result = _mm_shuffle_epi8(tangle, qangle);
        accumulate_u8_to_f32_avx2(result, score_ptr);
        // alignas(16) uint8_t result_array[16];
        //_mm_storeu_si128(reinterpret_cast<__m128i*>(result_array), result);
        // for (int n = 0; n < 16; ++n) {
        //   score_ptr[n] += static_cast<float>(result_array[n]);
        // }
      }
      for (int j = block_num * 16; j < valid_w; ++j) {
        uchar label = angle_row_ptr[j + x0];
        if (label == 255)
          continue;
        score_row_ptr[j] +=
            static_cast<float>((score_table_16d + lbl * 16)[label]);
      }
    }
  }
  score_map = score_map / 255 / fsz;
  return score_map;
}  // namespace shape_match
}  // namespace shape_match