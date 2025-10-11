#pragma once
#include <../include/armor.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
#include "../onnxruntime/include/onnxruntime_cxx_api.h" // 引入 ONNX Runtime
#include "../include/yolodetect.hpp"
#include "../include/videoReader.hpp"
class tools
{
public:
    static void drawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness);
    static cv::Mat enhanceContrast(const cv::Mat &inputFrame);
    static std::vector<std::vector<cv::Point>> getContours(cv::Mat &inputFrame,double minAreaThreshold);
    static cv::Mat adjustBrightness(const cv::Mat &inputFrame, int beta);
    static void drawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness);
    static cv::Mat mainFunction(const cv::Mat &inputFrame);
    static std::vector<cv::Mat> chopFrame(const std::vector<Armor> &inputArmors, const cv::Mat &inputFrame);
    static cv::Mat cropRotatedRect(cv::Mat &frame, const cv::RotatedRect &rRect);
    static void drawDetections(cv::Mat &img, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences);
    static void classifyArmors(long long &total, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences, std::vector<Robot> &cars, std::vector<Robot> &watchers, std::vector<Armor> &armors);
};