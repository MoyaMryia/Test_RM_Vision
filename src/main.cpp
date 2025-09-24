#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/video_reader.hpp"
#include "../include/video_processor.hpp"
//#include "video_capture.hpp"
//Step1 把assets/下面那个视频拉出来 使用opencv处理成图像然后挂事件
//done
//Step2 处理YOLO 
//stucked, FUCK.

//Step3 OPENCV confirm 装甲
//Step4 预测
int main(int argc, char** argv) {
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file_path>" << std::endl;
        return -1;
    } 
    //std::cout<<1;
    VideoReader reader(argv[1]);
    if (!reader.isOpened()) {
        std::cerr << "Error: Could not open the video file." << std::endl;
        return -1;
    }
    cv::Mat frame;
    cv::Mat processedFrame;
    while (true) {
        if (!reader.readFrame(frame)) {
            std::cout << "End of video stream or error occurred." << std::endl;
            break;
        }
        
        processedFrame = VideoProcessor::processFrame(frame);
        //cv::imshow("Original Video", frame);
        
        
        
        
        cv::imshow("Processed Video", processedFrame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27) {
            break;
        }
    }
    cv::destroyAllWindows();
    
    return 0;
}