#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
#include <onnxruntime/onnxruntime_cxx_api.h> // 引入 ONNX Runtime
#include "../include/yolodetect.hpp"
#include "../include/video_reader.hpp"
#include "../include/frameprocess.hpp"
#include "../include/tools.hpp"
// using namespace cv;
// using namespace std;

int main()
{
    // 1. 初始化和加载模型
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH))
    {
        return -1;
    }
    VideoReader reader(VIDEO_PATH);
    if (!reader.isOpened())
    {
        return -1;
    }
    cv::Mat frame;
    while (true)
    {
        if (!reader.readFrame(frame))
        {
            break;
        }
        // 3. 推理
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // std::cout << "Inference complete. Detected candidate boxes: " << output_data.size() / 6 << std::endl;

        // 4. 后处理
        std::vector<Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);

        // 5. 绘制结果
        YOLOv8Detector::draw_detections(frame, boxes, classIds, confidences);
        //std::cout << "Final detected objects after confidence filter: " << boxes.size() << std::endl;

        // 6. 显示结果
        cv::namedWindow("YOLOv8 Detection (ONNX Runtime)", cv::WINDOW_NORMAL);
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {
            break;
        }
    }
    cv::destroyAllWindows();

    return 0;
}