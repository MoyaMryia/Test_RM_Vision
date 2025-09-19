#include <iostream>
#include <opencv2/opencv.hpp>
#include "include/VideoReader.hpp"
#include "include/VideoProcessor.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file_path>" << std::endl;
        return -1;
    }

    // 1. 初始化 VideoReader
    VideoReader reader(argv[1]);

    if (!reader.isOpened()) {
        std::cerr << "Error: Could not open the video file." << std::endl;
        return -1;
    }

    cv::Mat frame;
    cv::Mat processedFrame;

    // 2. 主循环：读取、处理和显示
    while (true) {
        if (!reader.readFrame(frame)) {
            std::cout << "End of video stream or error occurred." << std::endl;
            break;
        }

        // 处理帧
        processedFrame = VideoProcessor::processFrame(frame);

        // 显示原始帧和处理后的帧
        cv::imshow("Original Video", frame);
        cv::imshow("Processed Video", processedFrame);

        // 按 'q' 或 ESC 键退出
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27) {
            break;
        }
    }

    cv::destroyAllWindows();
    
    return 0;
}