/****************************************
 * 模型推理类
 * 指定模型文件，执行推理
 ****************************************/

#pragma once
#include <string>
#include <vector>
#include "onnxruntime_cxx_api.h"
#include <opencv2/opencv.hpp>

#include "basic_algorithm_global.h"

 //模型输出，onnx 模型输出的尺寸为 BCHW,这里使用独立的内存存储其输出结果
struct  st_model_output
{
	int batch{ 0 }, channel{ 0 }, height{ 0 }, width{ 0 };
	float* data{ nullptr };
	st_model_output(int b = 0, int c = 0, int h = 0, int w = 0, float* input_data = nullptr)
		:batch(b), channel(c), height(h), width(w)
	{
		if (input_data != nullptr)
		{
			size_t total = static_cast<size_t>(b) * c * h * w;
			data = new float[total];
			memcpy_s(data, sizeof(float) * total, input_data, sizeof(float) * total);
		}
	}

	~st_model_output()
	{
		if (data != nullptr)
		{
			delete[]data;
			data = nullptr;
		}
	}
};

class BASIC_ALGORITHM_EXPORT predictor
{
public:
	predictor() = default;
	~predictor() = default;

	bool initialize(std::string model_path);

	bool predict(const cv::Mat& input, st_model_output& output) const;

	static bool check_same_data(const st_model_output& data, const std::string& ref_data_path);

private:
	Ort::Env m_env{ ORT_LOGGING_LEVEL_WARNING, "onnx_test" };

	std::unique_ptr<Ort::Session> m_session;
	std::vector<std::string> m_input_names;
	std::vector<std::string> m_output_names;
	std::vector<const char*> m_input_names_ptr;
	std::vector<const char*> m_output_names_ptr;
};
