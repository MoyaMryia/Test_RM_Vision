#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <map>
#include <filesystem> // C++17, 用于文件系统操作

namespace fs = std::filesystem;
using DigitTemplates = std::map<int, cv::Mat>;
struct MatchResult
{
    int digit = -1;
    double score = 0.0;
    cv::Rect location;
    double scale = 1.0;
};

class MatchNumber
{
public:
    static MatchResult recognizeSingleDigitByFeature(
        const cv::Mat &frame,
        const DigitTemplates &templates,
        int min_inlier_count = 10);
    static DigitTemplates loadTemplates(const std::string &template_dir, int mode);
};