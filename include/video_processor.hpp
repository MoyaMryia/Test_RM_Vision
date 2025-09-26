#pragma once

#include <opencv2/opencv.hpp>

class VideoProcessor {
public:
    static cv::Mat processFrame(const cv::Mat& inputFrame);
private:
    static void DrawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static bool chk_vaild(cv::RotatedRect &rotrect);
    static std::vector<cv::RotatedRect> findRect(const cv::Mat& inputFrame);
};