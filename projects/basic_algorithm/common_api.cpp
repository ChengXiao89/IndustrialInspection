#include "common_api.h"

cv::Mat convert_hwc_to_chw(const cv::Mat& src)
{
    int height = src.rows;
    int width = src.cols;
    int channels = src.channels();

    //1. 假设原始影像尺寸为 H*W*C，将之拆分成C个独立通道数据，每个数据尺寸为 H*W
    std::vector<cv::Mat> planes; 
    cv::split(src, planes);
    //2. 创建输出结果.这里创建的是一个尺寸为 new_height(等于原始数据中的元素总数)*1 的单通道数据(内存连续)，数据类型和 src 一致
    cv::Mat output = cv::Mat(channels * height * width, 1, src.depth());
	size_t elem_size = src.elemSize1();                 //返回单个元素的字节大小
    size_t channel_size = height * width * elem_size;   //返回每个通道中所有数据的字节大小
    //3. 将每个通道数据 planes[i].data 拷贝到 output[i*height * width]处
    // 这里得到的output中的数据存储方式为[C0C0C0.....C1C1C1......CNCNCN]
    for (int c = 0; c < channels; ++c)
    {
        std::memcpy(output.data + c * channel_size,
            planes[c].data,
            channel_size);
    }
    //4. 上面创建的 output 实际上是一维的内存连续数据，这里将之转换成三维数组
    //   转换之后的内存布局为
    /*  [
     *      [
     *          [D11,D12,......D1W]
     *          [D21,D22,......D2W]
     *          ....
     *          [DH1,DH2,......DHW]
     *      ]
     *      ...(共C个二维数组)
     *  ]
     */
    int sizes[3] = { channels, height, width };
    output = output.reshape(0, 3, sizes);
    return output;
}


cv::Mat convert_chw_to_hwc(const cv::Mat& src)
{
    if (src.dims != 3)
    {
        return cv::Mat();
    }
    //1. 输入数据为 C H W 布局，得到三个维度的尺寸
    int channels = src.size[0];
    int height = src.size[1];
    int width = src.size[2];
    //2. 创建一个二维数组，尺寸为H*W,同时数组中每个元素对应 C 个通道，数据类型与原始数据一致
    cv::Mat output(height, width, CV_MAKETYPE(src.depth(), channels));
    //3. 每个元素的字节数以及每个通道的字节数
    size_t elem_size = src.elemSize1();
    size_t channel_size = height * width * elem_size;
    //4. 将每个通道的数据存储在一个数组中，然后合并.数据交错存储的功能由cv::merge完成
    std::vector<cv::Mat> channels_vec;
    channels_vec.reserve(channels);
    for (int c = 0; c < channels; ++c)
    {
        const uchar* channel_data = src.data + c * channel_size;        //得到原始数据中各个通道数据
        cv::Mat channel_mat(height, width, src.depth(), (void*)channel_data);
        channels_vec.push_back(channel_mat.clone());
    }
    cv::merge(channels_vec, output);
    return output;
}

cv::Mat normalize_image(const cv::Mat& src, std::vector<float> mean, std::vector<float> std)
{
    cv::Mat img;
    src.convertTo(img, CV_32FC3, 1.0 / 255.0);
    if(mean.size() == src.channels() && std.size() == src.channels())
    {
        int channel_count = src.channels();
        std::vector<cv::Mat> channels(channel_count);
        cv::split(img, channels); // 分离通道
        for (int i = 0; i < channel_count; ++i)
        {
            channels[i] = (channels[i] - mean[i]) / std[i];
        }
        cv::merge(channels, img);
    }
    return img;
}

bool get_model_output_in_range(const st_model_output& model_output, int start_x, int start_y, int end_x, int end_y,
    std::vector<float>& output, int& batch, int& channel, int& height, int& width)
{
    if(model_output.data == nullptr || model_output.batch != 1 || model_output.channel == 0 ||
        model_output.height == 0 || model_output.width == 0)
    {
	    return false;
    }
    if(start_x > end_x || start_y > end_y)
    {
	    return false;
    }
    if(start_x >= model_output.width || start_y >= model_output.height || end_x <= 0 || end_y <= 0)
    {
	    return false;
    }
    start_x = std::max(0, start_x);
    start_y = std::max(0, start_y);
    end_x = std::min(end_x, model_output.width - 1);
    end_y = std::min(end_y, model_output.height - 1);
    //得到结果尺寸
    batch = model_output.batch;
    channel = model_output.channel;
    width = end_x - start_x + 1;
    height = end_y - start_y + 1;
    output.resize(batch * channel * height * width, 0.0);
    int dst_batch_step = channel * height * width;
    int dst_channel_step = height * width;
    int src_batch_step = model_output.channel * model_output.height * model_output.width;
    int src_channel_step = model_output.height * model_output.width;
    for (int b = 0;b < batch;b++)
    {
	    for (int c = 0;c < channel;c++)
	    {
		    for (int h = 0;h < height;h++)
		    {
			    for (int w = 0;w < width;w++)
			    {
                    int dst_index = b * dst_batch_step + c * dst_channel_step + h * width + w;
                    int src_index = b * src_batch_step + c * src_channel_step + (h + start_y) * model_output.width + (w + start_x);
                    output[dst_index] = model_output.data[src_index];
			    }
		    }
	    }
    }

	return true;
}



bool extract_sliding_window_features(const float* data, int batch, int channel, int height, int width, int kernel_size, int step_size, 
    std::vector<float>& features, int& feature_count_y, int& feature_count_x, int& dims)
{
	if(data == nullptr || batch != 1 || channel == 0 || height == 0 || width == 0)
	{
		return false;
	}
    if(kernel_size < 1 || step_size < 1 || height < kernel_size || width < kernel_size)
    {
	    return false;
    }
    const int out_h = (height - kernel_size) / step_size + 1;
    const int out_w = (width - kernel_size) / step_size + 1;

    /******************1.计算结果尺寸并分配内存*******************/
    feature_count_y = out_h;
    feature_count_x = out_w;
    int count = feature_count_y * feature_count_x;
    dims = channel * kernel_size * kernel_size;
    features.resize(count * dims, 0.0);

    /*************************2.滑动窗口展开**********************
     * 原始数据内存布局为 1*channel*height*width
     * 展开之后的内存布局为 1*channel*out_h*out_w*kernel_size*kernel_size
     * 需要填充的是out_h*out_w*kernel_size*kernel_size部分
     * 对于(r,c,i,j) in (0,0,0,0)-->(out_h,out_w,kernel_size,kernel_size),移动步长为step_size
     * 其在原始数据中的位置为[r*step_size+kernel_size/2+(i-kernel_size/2),c*step_size+kernel_size/2+(j-kernel_size/2)]
     * 化简可得[r*step_size + i,c*step_size + j]
     *************************************************************/
    int element_size = batch * channel;           //展开之后待填充子项的数量
    int element_step = out_h * out_w * kernel_size * kernel_size;           //展开之后待填充子项的尺寸
    for (int element_pos = 0; element_pos < element_size; element_pos++)
    {
        int start = element_pos * element_step;                                 //当前待填充项起点
        int start_src = element_pos * height * width; //当前待填充项在原始数中的起点
        for (int r = 0;r < out_h;r++)
        {
	        for (int c = 0;c < out_w;c++)
	        {
		        for (int i = 0;i < kernel_size;i++)
		        {
			        for (int j = 0;j < kernel_size;j++)
			        {
                        int dst_index = start + (((r * out_w + c) * kernel_size + i) * kernel_size + j);
                        int src_index = start_src + (r * step_size + i) * width + c * step_size + j;
                        features[dst_index] = data[src_index];
			        }
		        }
	        }
        }
    }
    /**************************3.数据重组***********************
     * 滑动展开之后需要进行数据重组，原始代码执行了三步操作:
     * features = features.contiguous().view(channel, -1, kernel_size, kernel_size)
     * features = features.permute(1, 0, 2, 3)
     * features = features.contiguous().view(features.shape[0], channel * kernel_size* kernel_size)
     * 在上一步中我们得到 features 的尺寸为batch * channel*out_h * out_w * kernel_size * kernel_size
     * 上述操作是将
     * (1) 将 features 尺寸重组为 channel*(out_h * out_w)*kernel_size * kernel_size（batch==1，这里不考虑）
     * (2) 继续将 features 尺寸重组为(out_h * out_w)*channel*kernel_size * kernel_size
     * (3) 最后将 features 尺寸重组为(out_h * out_w)*(channel*kernel_size * kernel_size)
     * 其中(1)与(3)不会改变数据，我们需要实现(2)
     * 现在我们将 features 尺寸解释为 channel*(out_h * out_w)*kernel_size * kernel_size
     * 按照如下步骤实现(2)转换 :
     * 2.1 结果中每个待填充子元素尺寸为 channel*kernel_size * kernel_size，共有 out_h * out_w 个子元素
     * 2.2 对于结果中每个子元素中的元素(ele,c,i,j):(0,0,0,0)-->(out_h * out_w,channel,kernel_size,kernel_size)
     * 其在features中的位置为c*(out_h * out_w)*kernel_size * kernel_size+ele*kernel_size * kernel_size+i**kernel_size+j
    *************************************************************/
    std::vector<float> result;
    result.resize(count * dims, 0.0);
    element_size = count;
    int kernel_step = kernel_size * kernel_size;
    element_step = channel * kernel_step;
    for (int index = 0; index < element_size; index++)
    {
	    for (int c = 0;c < channel;c++)
	    {
		    for (int i = 0;i < kernel_size;i++)
		    {
			    for (int j = 0;j < kernel_size;j++)
			    {
                    int dst_index = index * element_step + c * kernel_step + i * kernel_size + j;
                    int src_index = c * element_size * kernel_step + index * kernel_step + i * kernel_size + j;
                    result[dst_index] = features[src_index];
			    }
		    }
	    }
    }
    features = result;
    return true;
}

void normalize_vector_L2(float* input, int count, int dims)
{
	for (int i = 0;i < count;i++)
	{
        float* data = input + i * dims;
        double sum = 0.0;
        for (int j = 0;j < dims;j++)
        {
            sum += data[j] * data[j];
        }
        if(sum < 0.00001)
        {
	        continue;
        }
        double delta = 1.0 / sqrt(sum);
        for (int j = 0; j < dims; j++)
        {
            data[j] *= delta;
        }
	}
}

double get_iou(const st_detect_box& box1, const st_detect_box& box2)
{
    if(box1.m_x0 >= box1.m_x1 || box1.m_y0 >= box1.m_y1 || box2.m_x0 >= box2.m_x1 || box2.m_y0 >= box2.m_y1)
    {
        return 0.0;
    }
    double x0 = std::max(box1.m_x0, box2.m_x0);
    double y0 = std::max(box1.m_y0, box2.m_y0);
    double x1 = std::min(box1.m_x1, box2.m_x1);
    double y1 = std::min(box1.m_y1, box2.m_y1);

    if(x0 >= x1 || y0 >= y1)
    {
	    return 0.0;
    }
    double area1 = (box1.m_x1 - box1.m_x0) * (box1.m_y1 - box1.m_y0);
    double area2 = (box2.m_x1 - box2.m_x0) * (box2.m_y1 - box2.m_y0);
    double area_inter = (x1 - x0) * (y1 - y0);
    double area_union = area1 + area2 - area_inter;
    if(area_union < 0.0001)
    {
        return 0.0;
    }
    return area_inter / area_union;
}

bool is_circle_rect_intersect(const cv::Point& center, double radius, const cv::Rect& rect)
{
    double closest_x = std::clamp(center.x, rect.x, rect.x + rect.width);
    double closest_y = std::clamp(center.y, rect.y, rect.y + rect.height);
    double dx = center.x - closest_x;
    double dy = center.y - closest_y;
    return (dx * dx + dy * dy) < radius * radius;
}

cv::Mat get_roi_image(const cv::Mat& image,const st_detect_box& box, int extent, int channel, cv::Rect& roi)
{
    cv::Mat img;
    if (channel != 1 && channel != 3)
    {
        return img;
    }
    int x0 = std::max(0, static_cast<int>(box.m_x0) - extent);
    int y0 = std::max(0, static_cast<int>(box.m_y0) - extent);
    int x1 = std::min(image.cols, static_cast<int>(box.m_x1) + extent);
    int y1 = std::min(image.rows, static_cast<int>(box.m_y1) + extent);
    if (x0 >= x1 || y0 >= y1)
    {
        return img;
    }
    roi = cv::Rect(x0, y0, x1 - x0, y1 - y0);
    cv::Mat roi_image = image(roi);
    if (channel == 1)
    {
        cv::extractChannel(roi_image, img, 0);
    }
    else
    {
        img = roi_image;
    }
    return img;
}


void draw_boxes_to_image(const cv::Mat& image, const std::vector<std::vector<st_detect_box>>& boxes,const std::string& file_path)
{
    cv::Mat img_draw;
    if (image.channels() == 1)
        cv::cvtColor(image, img_draw, cv::COLOR_GRAY2RGB); // 单通道转 3 通道
    else
        img_draw = image.clone();

    cv::Scalar red = cv::Scalar(255, 0, 0);
    cv::Scalar green = cv::Scalar(0, 255, 0);
    for (int i = 0;i < boxes.size();i++)
    {
        const std::vector<st_detect_box>& box_array = boxes[i];
        for (int j = 0;j < box_array.size();j++)
        {
            const st_detect_box& box = box_array[j];
            cv::Point top_left(static_cast<int>(box.m_x0), static_cast<int>(box.m_y0));
            cv::Point bottom_right(static_cast<int>(box.m_x1), static_cast<int>(box.m_y1));
            if(i == 0)
            {
                cv::rectangle(img_draw, top_left, bottom_right, red, 2);
            }
            else
            {
                cv::rectangle(img_draw, top_left, bottom_right, green, 2);
            }
        }
    }
    // 保存图像
    cv::imwrite(file_path, img_draw);
}
