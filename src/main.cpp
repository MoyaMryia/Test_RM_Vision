#include "../include/tools.hpp"

int main()
{
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH))
    {
        return -1;
    }
    VideoReader reader(VIDEO_PATH);
    if (!reader.isOpened())
    {
        return -1;
    }
    cv::Mat frame;
    long long total = 1;
    while (true)
    {
        if (!reader.readFrame(frame))
        {
            // return 0;
            break;
        }
        // notice that I have alread adjusted contrasts
        // Noticed?
        // frame = tools::adjustBrightness(frame,20);
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

        // 后处理 的 一些定义
        std::vector<Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);
        std::vector<Armor> armors;
        std::vector<Robot> cars;
        std::vector<Robot> watchers;
        tools::classifyArmors(total, boxes, classIds, confidences, cars, watchers, armors);
        //
        std::vector<cv::Mat> outputFrames;
        outputFrames = tools::chopFrame(armors, frame);

        // 抠数字
        // contours里面包含一个抠出来的数字
        // RotateRect也有
        // 之后会转移到后处理函数里

        if (outputFrames.size() > 0)
        {
            for (int i = 0; i < outputFrames.size(); ++i)
            {
                std::vector<cv::Mat> channels;
                cv::split(outputFrames[i], channels);
                cv::Mat binary;
                cv::threshold(channels[classIds[i]], binary, 120, 255, cv::THRESH_BINARY);
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
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
                if(rotaterects[0].center.x>rotaterects[1].center.x) swap(rotaterects[1],rotaterects[0]);
                
                armors[i].Lightbars.left_LightBar = rotaterects[0];
                armors[i].Lightbars.right_LightBar = rotaterects[1];
            }
        }
        tools::drawDetections(frame, boxes, classIds, confidences);
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {
            break;
        }
    }
    cv::destroyAllWindows();
    // cv::waitKey(0);
    return 0;
}
/*
            for (auto &frame_try_2 : outputFrames_pre)
            {
                std::vector<cv::Mat> channels;
                cv::split(frame_try_2, channels);
                cv::Mat blue = channels.at(0);
                cv::Mat red = channels.at(2);
                cv::threshold(channels.at(0), blue, 120, 255, cv::THRESH_BINARY);
                cv::threshold(channels.at(2), red, 110, 255, cv::THRESH_BINARY);
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(red, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
                cv::Mat frame_try = frame_try_2.clone();
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
                tools::drawRotatedRect(frame_try, rotaterects[0], cv::Scalar(0, 255, 0), 1);
                tools::drawRotatedRect(frame_try, rotaterects[1], cv::Scalar(0, 255, 0), 1);
                cv::imshow("Chopped Colored", frame_try);
            }
*/