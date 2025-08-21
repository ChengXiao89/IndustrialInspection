#include <QtCore/QCoreApplication>

#include <iostream>
#include <QImage>
#include "../basic_algorithm/fiber_end_algorithm.h"
#include "cnpy.h"

cv::Mat QImage_to_cvMat(const QImage& image);
void test_detect_fiber_end(const std::string& input_path, const std::string& output_path);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    clock_t t1 = clock();
    test_detect_fiber_end("../../test_images/test_fiber_end.jpg", 
						"../../test_images/test_fiber_end_out.jpg");
    clock_t t2 = clock();

    std::cout << "use time " << t2 - t1 << " ms" << std::endl;
    return 0;
}

void test_detect_fiber_end(const std::string& input_path, const std::string& output_path)
{
    fiber_end_algorithm fiber_end_detector;
    if(!fiber_end_detector.initialize("./weights/fiber_end_model.onnx",
										"./faiss_index/faiss-binary_0",
									"./shape_model/model.bin"))
    {
        std::cout << "fiber_end_algorithm initialize error!" << std::endl;
        return;
    }
    QImage grayImage(QString::fromStdString(input_path));
    if (grayImage.isNull()) 
    {
        std::cout << "加载影像失败!" << std::endl;
        return;
    }
    // 2. 如果是单通道，转换成 RGB
    QImage rgbImage = grayImage.convertToFormat(QImage::Format_RGB888);
    //rgbImage.save("../../test_images/qimage.jpg");
    cv::Mat input_image = QImage_to_cvMat(rgbImage);
    if(DEBUG_LOCAL)
    {
        if(!basic_algorithm::check_same_data(input_image,"../../test_images/test_fiber_end_source_image.npy"))
        {
            std::cout << "source image_error!" << std::endl;
            return;
        }
    }
    //cv::imwrite("../../test_images/cvimage.jpg", input_image);
    fiber_end_detector.set_data(input_image);
    int ret = fiber_end_detector.run();
    if(ret == -1)
    {
    	std::cout << "search_fiber_end fail!" << std::endl;
        return;
    }
    else if(ret == -2)
    {
        std::cout << "shape_match fail!" << std::endl;
        return;
    }
    //获取检测结果
    const std::vector<std::vector<st_detect_box>>& result = fiber_end_detector.result();
    int count(0);
    for (int i = 0;i < result.size();i++)
    {
        count += result[i].size();
    }
    //如果检测到灰尘，保存到影像
    if(count > 0)
    {
        draw_boxes_to_image(input_image, result, "../../test_images/dust_detect_result.jpg");
    }



}

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