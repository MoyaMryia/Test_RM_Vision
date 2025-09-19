#pragma once

#include <string>
#include <opencv2/opencv.hpp>

class VideoReader {
public:
    VideoReader(const std::string& videoPath);
    ~VideoReader();
    
    bool isOpened() const;
    bool readFrame(cv::Mat& frame);

private:
    cv::VideoCapture cap;
};