#include "../include/detect.hpp"
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <iostream>

void DetectorCNN::YoloDetector(const std::string& model_path, float conf_threshold, float nms_threshold) { 
    net = cv::dnn::readNetFromONNX(model_path);
    if (net.empty()) {
        std::cerr << "Error: Could not load the ONNX model." << std::endl;
        // Handle error, e.g., throw an exception or exit
    }

    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); // Or DNN_TARGET_CUDA
}

std::vector<Armor> DetectorCNN::runInference(cv::Mat& frame) {
    // Prepare the frame for the model
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(), true, false);

    // Set the input to the network
    net.setInput(blob);

    // Run the forward pass
    std::vector<cv::Mat> outputs;
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    return processOutput(frame, outputs);
}

std::vector<Armor> DetectorCNN::processOutput(const cv::Mat& frame, const std::vector<cv::Mat>& outputs) {
    std::vector<Armor> detections;
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;

    cv::Mat output_data = outputs[0].reshape(1, outputs[0].size[2]);

    for (int i = 0; i < output_data.rows; ++i) {
        cv::Mat row = output_data.row(i);
        cv::Mat scores = row.colRange(4, 4 + num_classes);
        cv::Point class_id;
        double confidence;
        cv::minMaxLoc(scores, 0, &confidence, 0, &class_id);

        if (confidence > confidence_threshold) {
            float x_center = row.at<float>(0);
            float y_center = row.at<float>(1);
            float width = row.at<float>(2);
            float height = row.at<float>(3);

            int x = static_cast<int>((x_center - width / 2.0) * frame.cols / 640.0);
            int y = static_cast<int>((y_center - height / 2.0) * frame.rows / 640.0);
            int w = static_cast<int>(width * frame.cols / 640.0);
            int h = static_cast<int>(height * frame.rows / 640.0);

            detections.push_back({class_id.x, (float)confidence, cv::Rect(x, y, w, h)});
            boxes.push_back({x, y, w, h});
            confidences.push_back((float)confidence);
        }
    }

    // Perform NMS
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confidence_threshold, nms_threshold, indices);

    std::vector<Armor> final_detections;
    for (int idx : indices) {
        final_detections.push_back(detections[idx]);
    }

    return final_detections;
}