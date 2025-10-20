#pragma once
#include "include_all.hpp"

class tools
{
public:
    static void drawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness);
    static std::vector<std::vector<cv::Point>> getContours(cv::Mat &inputFrame,double minAreaThreshold);
    static void drawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static std::vector<cv::Mat> chopFrame(const std::vector<Armor> &inputArmors, const cv::Mat &inputFrame);
    static cv::RotatedRect getNormalizedRotatedRect_fortyfive(const cv::RotatedRect &rect);
    static bool is_pair_approx_equal(const Lightbar_Pair& pair1, const Lightbar_Pair& pair2);
    static cv::Rect bounding_rect_of_dual_rotated_rects(const Lightbar_Pair& dualRects);
    static void drawDetections(cv::Mat &img, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences);
    static void classifyArmors(long long &total, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences, std::vector<Robot> &cars, std::vector<Robot> &watchers, std::vector<Armor> &armors);
};