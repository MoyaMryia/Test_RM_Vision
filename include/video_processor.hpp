#pragma once

#include <opencv2/opencv.hpp>
#include "structures.hpp"
class VideoProcessor {
public:
    static cv::Mat processFrame(const cv::Mat& inputFrame);
private:
    static void DrawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static bool chk_vaild(cv::RotatedRect &rotrect);
    static std::vector<cv::RotatedRect> findRect(const cv::Mat& inputFrame);
    static std::vector<Lightbar_Pair> findArmor(std::vector<cv::RotatedRect> inputRects);
    static void DrawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness);
    static std::vector<Lightbar_Pair> findPairs(std::vector<cv::RotatedRect> inputRects,const cv::Mat& inputFrame);
};