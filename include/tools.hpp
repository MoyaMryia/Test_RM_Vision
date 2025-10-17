#pragma once
#include "include_all.hpp"
class tools
{
public:
    static void drawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness);
    static cv::Mat enhanceContrast(const cv::Mat &inputFrame);
    static std::vector<std::vector<cv::Point>> getContours(cv::Mat &inputFrame,double minAreaThreshold);
    static cv::Mat adjustBrightness(const cv::Mat &inputFrame, double beta);
    static void drawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static cv::Mat mainFunction(const cv::Mat &inputFrame);
    static std::vector<cv::Mat> chopFrame(const std::vector<Armor> &inputArmors, const cv::Mat &inputFrame);
    static cv::Mat cropRotatedRect(cv::Mat &frame, const cv::RotatedRect &rRect);
    static void drawDetections(cv::Mat &img, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences);
    static void classifyArmors(long long &total, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences, std::vector<Robot> &cars, std::vector<Robot> &watchers, std::vector<Armor> &armors);
};