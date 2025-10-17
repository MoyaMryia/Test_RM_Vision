#include "../include/include_all.hpp"
int main(int argc, char **argv)
{
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH))
    {
        return -1;
    }
    VideoReader reader(argv[1]);
    if (!reader.isOpened())
    {
        return -1;
    }
    cv::Mat frame;
    long long total = 1;
    double t0 = 0;                        // 用于存储上一次的 tick count
    double t_last = cv::getTickCount();   // 获取初始 tick count
    double freq = cv::getTickFrequency(); // 获取 tick 频率
    double fps = 0.0;
    while (true)
    {
        t0 = cv::getTickCount();
        if (!reader.readFrame(frame))
        {
            break;
        }

        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

        // 最后优化再拆分代码得了

        std::vector<Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);
        std::vector<Armor> armors;
        std::vector<Robot> cars;
        std::vector<Robot> watchers;
        tools::classifyArmors(total, boxes, classIds, confidences, cars, watchers, armors);
        std::vector<cv::Mat> outputFrames;
        outputFrames = tools::chopFrame(armors, frame);
        // Final at: Getlightbars
        if (outputFrames.size() > 0)
        {
            for (int i = 0; i < outputFrames.size(); ++i)
            {
                std::vector<cv::Mat> channels;
                cv::split(outputFrames[i], channels);
                cv::Mat binaryImage;
                cv::threshold(channels[armors[i].classId], binaryImage, 130, 255, cv::THRESH_BINARY);
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
                std::vector<cv::RotatedRect> rotaterects;
                for (const auto &contour : contours)
                {
                    auto rotaterect = cv::minAreaRect(contour);
                    rotaterects.emplace_back(rotaterect);
                }
                std::sort(rotaterects.begin(), rotaterects.end(),
                          [](const cv::RotatedRect &a, const cv::RotatedRect &b)
                          {
                              return a.size.area() > b.size.area();
                          });
                if (rotaterects.size()>1)
                {
                    if (rotaterects[0].center.x > rotaterects[1].center.x)
                        std::swap(rotaterects[0], rotaterects[1]);
                    armors[i].Lightbars.left_LightBar = rotaterects[0];
                    armors[i].Lightbars.right_LightBar = rotaterects[1];
                    armors[i].position = failbackFunc::GetArmorRect(outputFrames[i], armors[i].Lightbars);
                    // tools::drawLightbars(outputFrames[i],armors[i].Lightbars,cv::Scalar(0,255,0),2);
                }
                cv::imshow("output" + std::to_string(i), binaryImage);
            }
        }
        double t1 = cv::getTickCount();
        double time_per_frame = (t1 - t_last) / freq;

        // 计算帧率 (FPS)
        fps = 1.0 / time_per_frame;

        // 格式化 FPS 字符串
        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));

        // 在图像左上角显示 FPS
        cv::putText(frame,                    // 目标图像
                    fps_text,                 // 文本内容
                    cv::Point(10, 30),        // 文本位置 (x, y)
                    cv::FONT_HERSHEY_SIMPLEX, // 字体
                    1,                        // 字体大小 (scale)
                    cv::Scalar(0, 255, 0),    // 文本颜色 (B, G, R)
                    2);                       // 文本厚度 (thickness)

        // 更新上一次时间戳
        t_last = t1;
        // tools::drawDetections(frame, boxes, classIds, confidences);
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}