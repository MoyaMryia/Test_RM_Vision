#pragma once

#include <opencv2/opencv.hpp>

class VideoProcessor {
public:
    static cv::Mat processFrame(const cv::Mat& inputFrame);
    static void DrawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static bool chk_vaild(cv::RotatedRect &rotrect);
};