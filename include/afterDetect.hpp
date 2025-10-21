#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <map>
#include <filesystem> // C++17, 用于文件系统操作

namespace fs = std::filesystem;
using DigitTemplates = std::map<int, cv::Mat>;
struct MatchResult {
    int digit = -1;       // 识别出的数字
    double score = 0.0;   // 最高的匹配度
    cv::Rect location;    // 匹配区域的矩形
    double scale = 1.0;   // 最佳匹配时的缩放比例
};

class MatchNumber
{
public:
    static MatchResult recognizeSingleDigitByFeature(
    const cv::Mat& frame, 
    const DigitTemplates& templates, 
    double threshold = 0,
    double min_scale = 1.5,
    double max_scale = 15.0,
    double step_scale = 0.1
);
    static DigitTemplates loadTemplates(const std::string &template_dir, int mode);
};