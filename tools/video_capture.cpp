#include <iostream>
#include "video_capture.hpp"

bool VideoCaptureWrapper::isOpened() {
    return camera_raw.isOpened();
}

VideoCaptureWrapper::VideoCaptureWrapper(cv::String deviceID){
    camera_raw.open(deviceID);
    if(!camera_raw.isOpened()){
        std::cerr << "No Video" << std::endl;
    }
}
