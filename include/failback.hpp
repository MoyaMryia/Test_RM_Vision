#pragma once
#include "tools.hpp"
class failbackFunc{
    public:
        static std::vector<cv::Point2f> GetArmorRect(cv::Mat &image, Lightbar_Pair &inputPairs);
        static bool checkVaild(cv::RotatedRect inputRect);
        static std::vector<Lightbar_Pair> findPairs(std::vector<cv::RotatedRect> inputRects, const cv::Mat &inputFrame);
        static bool checkEnemy(cv::RotatedRect lightbar, cv::Mat frame);
};