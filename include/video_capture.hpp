#ifndef VIDEO_CAPTURE
#define VIDEO_CAPTURE

#include <opencv2/opencv.hpp>

class VideoCaptureWrapper{
    private:
        cv::VideoCapture camera_raw;
    public:
        VideoCaptureWrapper(cv::String deviceId);
        bool isOpened();
        bool readFrame(cv::Mat& frame);
};


#endif