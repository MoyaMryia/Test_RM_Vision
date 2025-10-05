#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "../include/video_reader.hpp"
#include "../include/frameprocess.hpp"
#include "../include/tools.hpp"
#include "../include/yolodetect.hpp"

void draw_detections(cv::Mat &frame, const std::vector<Detection> &detections, const std::vector<std::string> &class_list)
{
    for (const auto &det : detections)
    {
        // 绘制矩形框
        rectangle(frame, det.box, cv::Scalar(0, 255, 0), 2);

        // 构建标签文本
        std::string label = class_list[det.class_id] + cv::format(": %.2f", det.confidence);

        // 绘制标签背景
        int baseLine;
        cv::Size label_size = getTextSize(label, cv::HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.7, 1, &baseLine);
        int top = cv::max(det.box.tl().y, label_size.height);
        cv::rectangle(frame, cv::Point(det.box.tl().x, top - label_size.height),
                      cv::Point(det.box.tl().x + label_size.width, top + baseLine),
                      cv::Scalar(0, 255, 0), cv::LineTypes::FILLED);

        // 绘制标签文本
        cv::putText(frame, label, cv::Point(det.box.tl().x, top),
                    cv::HersheyFonts::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 1);
    }
}
int main(int argc, char **argv)
{

    if (argc < 4)
    {
        return -1;
    }
    VideoReader reader(argv[3]);
    const std::string model_path = argv[1];
    const std::string class_list_path = argv[2];
    if (!reader.isOpened())
    {
        return -1;
    }
    cv::Mat frame;
    YOLOv8 detector(model_path, class_list_path);
    std::vector<std::string> class_list;
    std::ifstream ifs(class_list_path);
    std::string line;
    while (getline(ifs, line))
    {
        class_list.push_back(line);
    }
    while (true)
    {
        if (!reader.readFrame(frame))
        {
            break;
        }
        std::vector<Detection> detections = detector.infer(frame);
        // cv::imshow("Originate Video", frame);
        // 5. 绘制检测结果
        draw_detections(frame, detections, class_list);
        cv::imshow("Originate Video", frame);
        std::cout << "Found " << detections.size() << " objects." << std::endl;
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}
