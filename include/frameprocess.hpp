#pragma once

#include <opencv2/opencv.hpp>
#include "structures.hpp"
class VideoProcessor
{
public:
    static cv::Mat processFrame(const cv::Mat &inputFrame);
    static cv::Mat processFrame_chopeed(const cv::Mat &inputFrame);
    
    static bool chk_vaild(cv::RotatedRect &rotrect);
    static std::vector<cv::RotatedRect> findRect(const cv::Mat &inputFrame);
    static std::vector<Lightbar_Pair> findArmor(std::vector<cv::RotatedRect> inputRects);

    static cv::RotatedRect GetArmorRect(cv::Mat &image, const Lightbar_Pair &inputPairs);
    static std::vector<Lightbar_Pair> findPairs(std::vector<cv::RotatedRect> inputRects, const cv::Mat &inputFrame);
    static cv::Mat cropRotatedRect(cv::Mat &frame, const cv::RotatedRect &rRect);
};