#include "../include/yolodetect.hpp"
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
