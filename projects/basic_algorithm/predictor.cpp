#include "predictor.h"

#include <filesystem>
#include <iostream>

#include "cnpy.h"

bool predictor::initialize(std::string model_path)
{
	if (!std::filesystem::exists(model_path))
	{
		std::cerr << "模型文件路径不存在...... " << model_path << std::endl;
		return false;
	}

	Ort::SessionOptions session_options;
	session_options.SetIntraOpNumThreads(1);
	session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
	try
	{
		std::wstring model_path_wstr(model_path.begin(), model_path.end());
		m_session = std::make_unique<Ort::Session>(m_env, model_path_wstr.c_str(), session_options);
	}
	catch (const Ort::Exception& e)
	{
		std::cerr << "加载模型文件失败...... " << e.what() << std::endl;
		return false;
	}

	//输入输出名称
	std::vector<std::string> input_names = m_session->GetInputNames();
	for (int i = 0; i < input_names.size(); i++)
	{
		m_input_names.push_back(input_names[i]);
		m_input_names_ptr.push_back(m_input_names.back().c_str());
	}

	std::vector<std::string> output_names = m_session->GetOutputNames();
	for (int i = 0; i < output_names.size(); i++)
	{
		m_output_names.push_back(output_names[i]);
		m_output_names_ptr.push_back(m_output_names.back().c_str());
	}
	return true;
}

bool predictor::predict(const cv::Mat& input, st_model_output& output) const
{
	// ONNX Runtime 内存分配器
	Ort::AllocatorWithDefaultOptions allocator;
	// 创建输入张量
	Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
	std::vector<int64_t> input_shape = { 1, input.size[0], input.size[1], input.size[2] };
	Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
		memory_info, reinterpret_cast<float*>(input.data), input.total(), input_shape.data(), input_shape.size());

	// 推理
	auto output_tensor = m_session->Run(Ort::RunOptions{ nullptr },
		m_input_names_ptr.data(), &input_tensor, 1,
		m_output_names_ptr.data(), m_output_names_ptr.size());

	// 获取输出
	float* output_data = output_tensor.front().GetTensorMutableData<float>();
	// 假设输出形状是 [1, C, H, W]
	Ort::TensorTypeAndShapeInfo output_info = output_tensor.front().GetTensorTypeAndShapeInfo();
	std::vector<int64_t> output_shape = output_info.GetShape();

	int batch = output_shape[0];
	int channel = output_shape[1];
	int height = output_shape[2];
	int width = output_shape[3];

	output.batch = batch;
	output.channel = channel;
	output.height = height;
	output.width = width;
	if(output.data != nullptr)
	{
		delete[]output.data;
	}
	output.data = new float[batch * channel * height * width];
	long long data_length = sizeof(float) * batch * channel * height * width;
	memcpy_s(output.data, data_length, output_data, data_length);
	return true;
}

bool predictor::check_same_data(const st_model_output& data, const std::string& ref_data_path)
{
	cnpy::NpyArray red_data = cnpy::npy_load(ref_data_path);
	float* ref_data_ptr = red_data.data<float>();
	std::vector<size_t> red_data_shape = red_data.shape;
	if (red_data_shape.size() != 4)
	{
		std::cout << "dim error!" << std::endl;
		return false;
	}
	if (red_data_shape[0] != data.batch || red_data_shape[1] != data.channel ||
		red_data_shape[2] != data.height || red_data_shape[3] != data.width)
	{
		std::cout << "shape error!" << std::endl;
		return false;
	}
	long long data_length = data.batch * data.channel * data.height * data.width;
	for (int i = 0; i < data_length; i++)
	{
		if (fabs(data.data[i] - ref_data_ptr[i]) > 0.0001)
		{
			std::cout << "data error!" << std::endl;
			return false;
		}
	}
	return true;
}