#include "../include/tools.hpp"

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
    while (true)
    {
        if (!reader.readFrame(frame))
        {
            break;
        }
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

        //最后优化再拆分代码得了
        
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
        if (outputFrames.size() > 0)
        {
            for (int i = 0; i < outputFrames.size(); ++i)
            {
                std::vector<cv::Mat> channels;
                cv::split(outputFrames[i], channels);
                cv::Mat binaryImage;
                cv::threshold(channels[armors[i].classId], binaryImage, 120, 255, cv::THRESH_BINARY);
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
                if(rotaterects[0].center.x>rotaterects[1].center.x)std::swap(rotaterects[0],rotaterects[1]);
                armors[i].Lightbars.left_LightBar = rotaterects[0];
                armors[i].Lightbars.right_LightBar = rotaterects[1];
                tools::drawLightbars(outputFrames[i],armors[i].Lightbars,cv::Scalar(0,255,0),2);
                cv::imshow("output"+i,outputFrames[i]);
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
    return 0;
}