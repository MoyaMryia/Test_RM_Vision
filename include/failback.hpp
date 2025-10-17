#pragma once
#include "tools.hpp"
class failbackFunc{
    public:
        static std::vector<cv::Point2f> GetArmorRect(cv::Mat &image, Lightbar_Pair &inputPairs);
        static cv::RotatedRect getNormalizedRotatedRect_fortyfive(const cv::RotatedRect &rect);
};