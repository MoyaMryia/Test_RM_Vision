#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/video_reader.hpp"
#include "../include/frameprocess.hpp"
#include "../include/tools.hpp"

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <video_file_path>" << std::endl;
        return -1;
    }
    VideoReader reader(argv[1]);
    if (!reader.isOpened()){
            return -1;
    }
    cv::Mat frame;
    while (true){
        if (!reader.readFrame(frame)){
            break;
        }
        cv::imshow("Chopped Video", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27){
                break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}
