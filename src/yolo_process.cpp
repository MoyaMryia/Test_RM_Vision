#include "yolodetect.hpp"
#include <fstream>
#include <iostream>

using namespace cv;
using namespace cv::dnn;
using namespace std;

// 构造函数实现（不变）
YOLOv8::YOLOv8(const std::string& model_path, const std::string& class_list_path) {
    // 1. 加载模型
    try {
        net = readNetFromONNX(model_path);
        net.setPreferableBackend(DNN_BACKEND_OPENCV);
        net.setPreferableTarget(DNN_TARGET_CPU); 
    } catch (const cv::Exception& e) {
        cerr << "Error loading ONNX model: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    
    // 2. 加载类别名称
    load_class_list(class_list_path);
}

// 加载类别名称文件实现（不变）
void YOLOv8::load_class_list(const std::string& class_list_path) {
    // ... (不变)
}

// 推理函数实现
vector<Detection> YOLOv8::infer(Mat& image) {
    // 1. 预处理：创建4D blob
    Mat blob;
    // 使用新的 1280x1280 尺寸来创建 blob
    blobFromImage(image, blob, 1.0/255.0, Size(INPUT_WIDTH, INPUT_HEIGHT), Scalar(), true, false); 
    
    net.setInput(blob);

    // 2. 前向传播（推理）
    vector<Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    // 3. 后处理
    vector<Detection> detections;
    post_process(image, outputs[0], detections);

    return detections;
}

// 后处理函数实现
void YOLOv8::post_process(Mat& frame, const Mat& output, vector<Detection>& detections) {
    Mat data_T = output.reshape(1, output.size[1]);
    Mat detection_data = data_T.t();

    const int dimensions = detection_data.cols;
    const int rows = detection_data.rows;
    const int num_classes = dimensions - 4; 

    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;

    // 关键点：这里会使用 frame.cols (1280) 和 INPUT_WIDTH (1280)
    float x_factor = (float)frame.cols / INPUT_WIDTH;
    // 关键点：这里会使用 frame.rows (1020) 和 INPUT_HEIGHT (1280)
    float y_factor = (float)frame.rows / INPUT_HEIGHT;
    
    // ... (后续的循环和 NMS 逻辑不变)
    for (int i = 0; i < rows; ++i) {
        float* row_ptr = detection_data.ptr<float>(i);
        
        Mat scores(1, num_classes, CV_32F, row_ptr + 4);
        
        Point class_id_point;
        double max_confidence;
        minMaxLoc(scores, 0, &max_confidence, 0, &class_id_point);
        
        if (max_confidence > SCORE_THRESHOLD) {
            confidences.push_back((float)max_confidence);
            class_ids.push_back(class_id_point.x);
            
            float cx = row_ptr[0];
            float cy = row_ptr[1];
            float w = row_ptr[2];
            float h = row_ptr[3];
            
            // 转换为 (左上角x, 左上角y, 宽度, 高度)
            int left = (int)((cx - 0.5f * w) * x_factor);
            int top = (int)((cy - 0.5f * h) * y_factor);
            int width = (int)(w * x_factor);
            int height = (int)(h * y_factor);
            
            boxes.push_back(Rect(left, top, width, height));
        }
    }

    vector<int> indices;
    NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, indices);

    for (size_t i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        Detection det;
        det.class_id = class_ids[idx];
        det.confidence = confidences[idx];
        det.box = boxes[idx];
        detections.push_back(det);
    }
}