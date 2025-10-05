#ifndef YOLOV8_H
#define YOLOV8_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// 定义一个结构体来存储检测结果
struct Detection {
    int class_id;
    float confidence;
    cv::Rect box;
};

class YOLOv8 {
public:
    // 构造函数：加载模型和类别名称
    YOLOv8(const std::string& model_path, const std::string& class_list_path);

    // 执行推理
    std::vector<Detection> infer(cv::Mat& image);

private:
    cv::dnn::Net net;
    std::vector<std::string> class_list;

    // *** 关键修改：匹配您导出的 ONNX 模型的输入尺寸 ***
    const int INPUT_WIDTH = 1280;   
    const int INPUT_HEIGHT = 1280;  
    
    const float SCORE_THRESHOLD = 0.25f; 
    const float NMS_THRESHOLD = 0.45f;   

    // 后处理函数（从ONNX输出中解析检测结果）
    void post_process(cv::Mat& frame, const cv::Mat& output, std::vector<Detection>& detections);
    
    // 加载类别名称文件
    void load_class_list(const std::string& class_list_path);
};

#endif // YOLOV8_H