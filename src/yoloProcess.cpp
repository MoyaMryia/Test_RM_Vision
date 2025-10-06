#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
#include <onnxruntime/onnxruntime_cxx_api.h> // 引入 ONNX Runtime
#include "../include/yolodetect.hpp"
using namespace cv;
using namespace std;

void YOLOv8Detector::post_process_ort(Mat &frame, const vector<float> &output_data,
                                      vector<Rect> &boxes, vector<int> &classIds, vector<float> &confidences)
{
    const int num_elements = 6;

    size_t total_elements = output_data.size();
    if (total_elements % num_elements != 0)
    {
        return;
    }
    int num_detections = total_elements / num_elements;

    for (int i = 0; i < num_detections; ++i)
    {
        size_t offset = i * num_elements;

        float x1 = output_data[offset + 0];
        float y1 = output_data[offset + 1];
        float x2 = output_data[offset + 2];
        float y2 = output_data[offset + 3];
        float confidence = output_data[offset + 4];
        int class_id = static_cast<int>(output_data[offset + 5]);

        if (confidence >= CONFIDENCE_THRESHOLD)
        {
            int left = static_cast<int>(x1 * frame.cols / INPUT_WIDTH);
            int top = static_cast<int>(y1 * frame.rows / INPUT_HEIGHT);
            int right = static_cast<int>(x2 * frame.cols / INPUT_WIDTH);
            int bottom = static_cast<int>(y2 * frame.rows / INPUT_HEIGHT);

            boxes.emplace_back(left, top, right - left, bottom - top);
            classIds.push_back(class_id);
            confidences.push_back(confidence);
        }
    }
}
