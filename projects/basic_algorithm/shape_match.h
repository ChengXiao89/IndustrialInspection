#pragma once
#include <immintrin.h>
#include <omp.h>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

namespace shape_match {

constexpr unsigned char score_table_16d[16 * 16] = {
    255, 247, 230, 203, 172, 135, 93,  52,  0,   52,  93,  135, 172, 203, 230,
    247, 247, 255, 247, 230, 203, 172, 135, 93,  52,  0,   52,  93,  135, 172,
    203, 230, 230, 247, 255, 247, 230, 203, 172, 135, 93,  52,  0,   52,  93,
    135, 172, 203, 203, 230, 247, 255, 247, 230, 203, 172, 135, 93,  52,  0,
    52,  93,  135, 172, 172, 203, 230, 247, 255, 247, 230, 203, 172, 135, 93,
    52,  0,   52,  93,  135, 135, 172, 203, 230, 247, 255, 247, 230, 203, 172,
    135, 93,  52,  0,   52,  93,  93,  135, 172, 203, 230, 247, 255, 247, 230,
    203, 172, 135, 93,  52,  0,   52,  52,  93,  135, 172, 203, 230, 247, 255,
    247, 230, 203, 172, 135, 93,  52,  0,   0,   52,  93,  135, 172, 203, 230,
    247, 255, 247, 230, 203, 172, 135, 93,  52,  52,  0,   52,  93,  135, 172,
    203, 230, 247, 255, 247, 230, 203, 172, 135, 93,  93,  52,  0,   52,  93,
    135, 172, 203, 230, 247, 255, 247, 230, 203, 172, 135, 135, 93,  52,  0,
    52,  93,  135, 172, 203, 230, 247, 255, 247, 230, 203, 172, 172, 135, 93,
    52,  0,   52,  93,  135, 172, 203, 230, 247, 255, 247, 230, 203, 203, 172,
    135, 93,  52,  0,   52,  93,  135, 172, 203, 230, 247, 255, 247, 230, 230,
    203, 172, 135, 93,  52,  0,   52,  93,  135, 172, 203, 230, 247, 255, 247,
    247, 230, 203, 172, 135, 93,  52,  0,   52,  93,  135, 172, 203, 230, 247,
    255};

struct Feature {
  int x;
  int y;
  int lbl;      // 量化后的角度特征
  float angle;  // 原始角度
};

struct Candidate {
  /// Sort candidates with high score to the front
  float score;
  Feature feature;
  bool operator<(const Candidate& rhs) const { return score > rhs.score; };
};

struct Template {
  int id = 0;
  int pyramid_level = 0;
  int is_valid = 0;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  float angle = 0.0f;
  float scale = 1.0f;
  std::vector<Feature> features;
};

class TemplData {
 public:
  cv::Mat image;
  float start_angle, stop_angle, step_angle;
  float start_scale, stop_scale, step_scale;
  std::vector<float> pyramid_angle_steps;
  std::vector<float> pyramid_scale_steps;
  std::vector<float> pyramid_thresholds;
  int pyramid_levels;
  std::vector<std::vector<Template>> templates_list;
  cv::Point2f template_original_center;
  cv::Rect rect_roi;
  // 保存与加载函数
  void save(const std::string& filename) const;
  void load(const std::string& filename);
  void auto_calculate_pyramid_levels();
  void auto_calculate_angle_step();
  void auto_calculate_scale_step();
  void auto_calculate_thresholds(std::vector<cv::Mat>& imgs);
  void set_original_center(float x, float y);
};

struct MatchResult {
  int x;
  int y;
  int w;
  int h;
  float angle;
  float scale;
  std::vector<float> transform_matrix;  // 变换矩阵（2x3）
  float similarity;
  int template_id;
  std::vector<Feature> features;
  bool operator<(const MatchResult& other) const {
    if (similarity != other.similarity)
      return similarity > other.similarity;
    return template_id < other.template_id;
  }
};

TemplData create_shape_model(const cv::Mat& img,
                             float start_angle = 0,
                             float stop_angle = 0,
                             float step_angle = -1,
                             float start_scale = 1.0f,
                             float stop_scale = 1.0f,
                             float step_scale = 1.0f,
                             int pyramid_levels = 0,
                             int num_features = 400,
                             cv::Mat mask = cv::Mat());

TemplData read_shape_model(const std::string& filename);

std::vector<MatchResult> find_shape_model(const cv::Mat& img,
                                          TemplData templ_data,
                                          cv::Rect rect = cv::Rect(0, 0, 0, 0),
                                          float score_threshold = 0.6f,
                                          float overlap_threshold = 0.1f,
                                          int max_results = 100,
                                          float mag_thresh = 50.0f,
                                          int pyramid_level = -1);
void draw_shape_results(const cv::Mat& image,
                        const std::vector<MatchResult>& results,
                        const cv::Scalar& color = cv::Scalar(0, 255, 0),
                        int thickness = 2);
Template ExtractTemplate(cv::Mat angle,
                         cv::Mat quantized_angle,
                         cv::Mat mag,
                         int pyr_level,
                         float weak_thresh,
                         float strong_thresh,
                         int num_features,
                         cv::Mat mask);
std::vector<int> SelectScatteredFeatures(std::vector<Candidate>& candidates,
                                         int num_features);
void CropTemplate(Template& templ);
void AddTemplate_Transform(Template& templ,
                           float angle,
                           float scale,
                           bool calc_180);
cv::Mat Matching_16(cv::Mat& quantized_angle, const Template& templ);
cv::Mat Matching_16_simd(cv::Mat& quantized_angle, const Template templ);
std::vector<MatchResult> doNMSMatches(const std::vector<MatchResult>& matches,
                                      float score_thresh,
                                      float overlap_thresh);
std::vector<MatchResult> GetTopKMatches(std::vector<MatchResult>& matches,
                                        int top_k_);

}  // namespace shape_match
