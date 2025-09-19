#pragma once

#include <opencv2/opencv.hpp>

class VideoProcessor {
public:
    static cv::Mat processFrame(const cv::Mat& inputFrame);
};