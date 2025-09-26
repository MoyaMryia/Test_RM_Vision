#pragma once

#include <opencv2/opencv.hpp>

class detectNum{
    public:
        static cv::Mat enhanceContrast(const cv::Mat& inputFrame);
        static cv::Mat Mainfunction(const cv::Mat& inputFrame);
        static void grayandbinary(cv::Mat& inputFrame);
        static cv::Mat adjustBrightness(const cv::Mat& inputFrame, int beta);
};