#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/detect.hpp"
#include "../include/video_reader.hpp"
#include "../include/video_processor.hpp"
#include "../include/detect_num.hpp"
// #include "video_capture.hpp"
// Step1 把assets/下面那个视频拉出来 使用opencv处理成图像然后挂事件
// done
// Step2 处理YOLO
// stucked, FUCK.

// Step3 OPENCV confirm 装甲
// Step4 预测

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <video_file_path>" << std::endl;
        return -1;
    }
    while (1)
    {
        VideoReader reader(argv[1]);
        if (!reader.isOpened())
        {
            return -1;
        }
        cv::Mat frame;
        cv::Mat processedFrame;
        cv::Mat processedFrame_chopped;
        cv::Mat processedFrame_chopped_out;
        // DetectorCNN Dect;
        // Dect.YoloDetector("models");
        bool ds = 0;
        while (true)
        {

            if (!reader.readFrame(frame))
            {
                break;
            }
            processedFrame = frameProcess::processFrame(frame);
            processedFrame_chopped = frameProcess::processFrame_chopeed(frame);
            processedFrame_chopped_out = detectNum::Mainfunction(processedFrame_chopped);
            cv::imshow("Chopped Video", processedFrame_chopped_out);
            cv::imshow("Processed Video", processedFrame);
            if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
            {
                ds = 1;
                break;
            }
        }
        if (ds)
            break;
    }
    cv::destroyAllWindows();
    return 0;
}