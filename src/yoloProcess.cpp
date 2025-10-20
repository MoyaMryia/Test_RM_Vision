#include "../include/yolodetect.hpp"
#include "../include/tools.hpp"

using namespace cv;
using namespace std;

void YOLOv8Detector::post_process_ort(Mat &frame, const vector<float> &output_data,
                                      vector<Rect> &boxes, vector<int> &classIds, vector<float> &confidences)
{
    const int num_elements = 6;

    size_t total_elements = output_data.size();
    if (total_elements % num_elements != 0)
    {
        return;
    }
    int num_detections = total_elements / num_elements;

    for (int i = 0; i < num_detections; ++i)
    {
        size_t offset = i * num_elements;

        float x1 = output_data[offset + 0];
        float y1 = output_data[offset + 1];
        float x2 = output_data[offset + 2];
        float y2 = output_data[offset + 3];
        float confidence = output_data[offset + 4];
        int class_id = static_cast<int>(output_data[offset + 5]);

        if (confidence >= CONFIDENCE_THRESHOLD)
        {
            int left = static_cast<int>(x1 * frame.cols / INPUT_WIDTH);
            int top = static_cast<int>(y1 * frame.rows / INPUT_HEIGHT);
            int right = static_cast<int>(x2 * frame.cols / INPUT_WIDTH);
            int bottom = static_cast<int>(y2 * frame.rows / INPUT_HEIGHT);
            left = std::max(0, left);
            top = std::max(0, top);
            right = std::min(frame.cols, right);
            bottom = std::min(frame.rows, bottom);
            if ((right - left) > 0 && (bottom - top) > 0)
            {
                if (((((bottom - top) * (right - left) < 162500) || ((((bottom - top)) * 1.000 / ((right - left) * 1.000)) < 1.1)) && ((bottom - top) * (right - left) < 600000)))
                {
                    boxes.emplace_back(left, top, right - left, bottom - top);
                    classIds.push_back(class_id);
                    confidences.push_back(confidence);
                }
            }
        }
    }
}

std::vector<Armor> YOLOv8Detector::mainFunction(Mat frame, YOLOv8Detector &detector, long long &total)
{
    std::vector<Armor> armorsYolo;
    std::vector<float> output_data = detector.preprocessAndInference(frame);
    // output_data的格式如下：
    // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

    // 最后优化再拆分代码得了

    std::vector<Rect> boxes;
    std::vector<int> classIds;
    std::vector<float> confidences;
    post_process_ort(frame, output_data, boxes, classIds, confidences);

    std::vector<Robot> cars_mid;
    std::vector<Robot> watchers_mid;
    tools::classifyArmors(total, boxes, classIds, confidences, cars_mid, watchers_mid, armorsYolo);
    std::vector<cv::Mat> outputFrames;
    outputFrames = tools::chopFrame(armorsYolo, frame);

    // Final at: afterDetections::Getlightbars
    if (outputFrames.size() > 0)
    {
        for (int i = 0; i < outputFrames.size(); ++i)
        {
            std::vector<cv::Mat> channels;
            cv::split(outputFrames[i], channels);
            cv::Mat binaryImage;
            cv::Mat preImage = channels[_ENEMY].clone();
            cv::threshold(preImage, binaryImage, 130, 255, cv::THRESH_BINARY);
            cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
            cv::dilate(binaryImage, binaryImage, element);
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
            if (rotaterects.size() > 1)
            {
                if (rotaterects[0].center.x > rotaterects[1].center.x)
                    std::swap(rotaterects[0], rotaterects[1]);
                rotaterects[0].center.x += armorsYolo[i].Box.x;
                rotaterects[0].center.y += armorsYolo[i].Box.y;
                rotaterects[1].center.x += armorsYolo[i].Box.x;
                rotaterects[1].center.y += armorsYolo[i].Box.y;
                armorsYolo[i].Lightbars.left_LightBar = rotaterects[0];
                armorsYolo[i].Lightbars.right_LightBar = rotaterects[1];
                // armorsYolo[i].position = failbackFunc::GetArmorRect(outputFrames[i], armorsYolo[i].Lightbars);
                // tools::drawLightbars(outputFrames[i], armorsYolo[i].Lightbars, cv::Scalar(255, 0, 255), 2);
            }
            // tools::drawDetections(frame, boxes, classIds, confidences);
            // cv::imshow("output_binary" + std::to_string(i), binaryImage);
            // cv::imshow("output_pre" + std::to_string(i), preImage);
        }
    }
    return armorsYolo;
}