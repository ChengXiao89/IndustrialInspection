#include <QtCore/QCoreApplication>
#include <iostream>
#include <vector>
#include <onnxruntime_cxx_api.h>  // C++ API
#include <QDebug>

#include "cnpy.h"

void test_onnxruntime();

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    test_onnxruntime();
    return app.exec();
}

void test_onnxruntime()
{
    // 1. 初始化环境
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "onnx_test");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // 2. 创建 session
    std::string model_path = "../../weights/resnet18_layer3.onnx";
    std::wstring model_path_wstr(model_path.begin(), model_path.end());
    Ort::Session session(env, model_path_wstr.c_str(), session_options);
    //Ort::Session session(env, "../../weights/resnet18_layer3.onnx", session_options);

    // 3. 获取模型输入信息
    Ort::AllocatorWithDefaultOptions allocator;

    auto input_shape = session.GetInputTypeInfo(0)
        .GetTensorTypeAndShapeInfo()
        .GetShape();
    std::cout << "Input shape: ";
    for (auto dim : input_shape) std::cout << dim << " ";
    std::cout << std::endl;

    // 4. 构造输入数据 (1x3x120x500)
    cnpy::NpyArray image = cnpy::npy_load("image.npy");
    float* input_data = image.data<float>();
    std::vector<size_t> shape = image.shape;
    std::vector<int64_t> input_dims(shape.begin(), shape.end());

    // 5. 创建 ONNX Runtime Tensor
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, input_data, image.num_vals,
        input_dims.data(), input_dims.size()
    );

    // 6. 推理
    std::vector<std::string> input_names = session.GetInputNames();
    std::vector<const char*> input_names1;
    for (int i = 0; i < input_names.size(); i++)
    {
        std::cout << i << "......" << input_names[i] << std::endl;
        input_names1.push_back(input_names[i].c_str());
    }
    std::vector<std::string> output_names = session.GetOutputNames();
    std::vector<const char*> output_names1;
    for (int i = 0; i < output_names.size(); i++)
    {
        std::cout << i << "......" << output_names[i] << std::endl;
        output_names1.push_back(output_names[i].c_str());
    }

    auto output_tensors = session.Run(Ort::RunOptions{ nullptr },
        input_names1.data(), & input_tensor, 1,
        output_names1.data(), 1);

    // 7. 读取输出
    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    std::cout << "Output shape: ";
    for (auto dim : output_shape) std::cout << dim << " ";
    std::cout << std::endl;
    // 8. 对比结果
    cnpy::NpyArray feature_map = cnpy::npy_load("feature_map.npy");
    float* ref_data = feature_map.data<float>();
    std::vector<size_t> feature_map_shape = feature_map.shape;
    if(feature_map_shape.size() != output_shape.size())
    {
        qDebug() << QString::fromStdString("输出结果维度与参考结果维度不一致!");
        return;
    }
    for (int i = 0;i < feature_map_shape.size();i++)
    {
	    if(feature_map_shape[i] != output_shape[i])
	    {
            qDebug() << QString::fromStdString("输出结果形状与参考结果形状不一致!");
            return;
	    }
    }
    for (int i = 0;i < feature_map.num_vals;i++)
    {
	    if(fabs(output_data[i]- ref_data[i]) > 0.0001)
	    {
            qDebug() << QString::fromStdString("输出结果与参考结果不一致!");
            return;
	    }
    }
    qDebug() << QString::fromStdString("推理完成，结果正常!");
}