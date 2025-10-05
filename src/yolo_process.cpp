#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory> // for std::unique_ptr
#include <opencv2/opencv.hpp> // 引入 OpenCV 用于图像处理和绘制
#include <onnxruntime/onnxruntime_cxx_api.h> // 引入 ONNX Runtime
#include "../include/yolodetect.hpp"
using namespace cv;
using namespace std;
const vector<string> CLASS_NAMES = {
    "armor_blue", "armor_grey", "armor_red", 
    "car_blue", "car_red", "car_unknown", 
    "watcher_blue", "watcher_red", "watcher_unknown"
};
void YOLOv8Detector::draw_detections(Mat& img, const vector<Rect>& boxes, const vector<int>& classIds, const vector<float>& confidences) {
    for (size_t i = 0; i < boxes.size(); ++i) {
        rectangle(img, boxes[i], Scalar(0, 255, 0), 2);
        string label = CLASS_NAMES[classIds[i]] + format(": %.2f", confidences[i]);

        int baseLine;
        Size label_size = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int top = boxes[i].tl().y;
        rectangle(img, Point(boxes[i].tl().x, top - label_size.height - baseLine),
                  Point(boxes[i].tl().x + label_size.width, top),
                  Scalar(0, 255, 0), FILLED);

        putText(img, label, Point(boxes[i].tl().x, top - baseLine),
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 1);
    }
}

void YOLOv8Detector::post_process_ort(Mat& frame, const vector<float>& output_data, 
                      vector<Rect>& boxes, vector<int>& classIds, vector<float>& confidences) {
    const int num_elements = 6; 
    
    size_t total_elements = output_data.size();
    if (total_elements % num_elements != 0) {
        cerr << "Warning: Output size is not a multiple of " << num_elements << ". Total elements: " << total_elements << endl;
        return;
    }
    int num_detections = total_elements / num_elements;

    for (int i = 0; i < num_detections; ++i) {
        size_t offset = i * num_elements;
        
        float x1 = output_data[offset + 0];
        float y1 = output_data[offset + 1];
        float x2 = output_data[offset + 2];
        float y2 = output_data[offset + 3];
        float confidence = output_data[offset + 4];
        int class_id = static_cast<int>(output_data[offset + 5]); 

        if (confidence >= CONFIDENCE_THRESHOLD) {
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

