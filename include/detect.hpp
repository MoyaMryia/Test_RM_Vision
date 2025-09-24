#ifndef DETECT_HPP
#define DETECT_HPP

#include <opencv2/opencv.hpp>
#include "structures.hpp"
#include "armor.hpp"
#include <iostream>
class DetectorCNN{
public:
    // Constructor to load the model
    void YoloDetector(const std::string& model_path, float conf_threshold = 0.45, float nms_threshold = 0.5);

    // Run detection on a single image frame
    std::vector<Armor> runInference(cv::Mat& frame);

private:
    cv::dnn::Net net;
    float confidence_threshold;
    float nms_threshold;
    int num_classes; // You can make this configurable if needed

    // Internal method to process model outputs
    std::vector<Armor> processOutput(const cv::Mat& frame, const std::vector<cv::Mat>& outputs);
};

#endif