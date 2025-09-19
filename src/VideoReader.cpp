#include "../include/VideoReader.hpp"

VideoReader::VideoReader(const std::string& videoPath) {
    cap.open(videoPath);
}

VideoReader::~VideoReader() {
    if (cap.isOpened()) {
        cap.release();
    }
}

bool VideoReader::isOpened() const {
    return cap.isOpened();
}

bool VideoReader::readFrame(cv::Mat& frame) {
    return cap.read(frame);
}