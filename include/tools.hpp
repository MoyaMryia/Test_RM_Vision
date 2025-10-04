#pragma once

#include <opencv2/opencv.hpp>
#include <../include/armor.hpp>

class tools
{
public:
    static void DrawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness);
    static cv::Mat enhanceContrast(const cv::Mat &inputFrame);
    static std::vector<std::vector<cv::Point>> binaryimages(cv::Mat &inputFrame);
    static cv::Mat adjustBrightness(const cv::Mat &inputFrame, int beta);
    static void DrawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static cv::Mat Mainfunction(const cv::Mat &inputFrame);
};