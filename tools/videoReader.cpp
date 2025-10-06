#include "../include/videoReader.hpp"

VideoReader::VideoReader(const std::string &videoPath)
{
    cap.open(videoPath);
    // cap.open(0);
}

VideoReader::~VideoReader()
{
    if (cap.isOpened())
    {
        cap.release();
    }
}

bool VideoReader::isOpened() const
{
    return cap.isOpened();
}

bool VideoReader::readFrame(cv::Mat &frame)
{
    return cap.read(frame);
}