#include "basic_algorithm.h"
#include "cnpy.h"

bool basic_algorithm::check_same_data(const cv::Mat& data, const std::string& ref_data_path)
{
    int height = data.rows;
    int width = data.cols;
    int channels = data.channels();
    // 如果不是 float 类型，需要转换
    cv::Mat data_float;
    data.convertTo(data_float, CV_32F);
    //读取的一维数组
    if (!data_float.isContinuous())
        data_float = data_float.clone();
    // 展平为一维数组，不改变通道顺序
    std::vector<float> flat_data((float*)data_float.datastart, (float*)data_float.dataend);
    //加载参考数据
    cnpy::NpyArray ref_data = cnpy::npy_load(ref_data_path);
    float* ref_data_ptr = ref_data.data<float>();

    std::vector<size_t> ref_shape = ref_data.shape;
    long long  length(1);
    for (int i = 0; i < ref_shape.size(); i++)
    {
        length *= ref_shape[i];
    }
    if (length != flat_data.size())
    {
        std::cout << "size error" << std::endl;
        return false;
    }
    for (int i = 0; i < length; i++)
    {
        if (fabs(ref_data_ptr[i] - flat_data[i]) > 0.0001)
        {
            std::cout << "data error" << std::endl;
            return false;
        }
    }
    return true;
}
