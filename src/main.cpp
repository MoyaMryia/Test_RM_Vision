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
    //while (true)
    //{
        if (!reader.readFrame(frame))
        {
            return 0;
            //break;
        }
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

        // 后处理
        std::vector<Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);
        std::vector<Armor> armors;
        std::vector<Robot> cars;
        std::vector<Robot> watchers;
        std::vector<cv::Mat> outputFrames;
        //outputFrames = tools::chopFrame(armors, frame); 
        // There should be something that can be treated;
        // A. afterDetections::mainCalculations
        // B. create a std::vector<Robot> and take the armors into it;
        // C. Kalman Calculations
        // D. And finally, get a mark, done.
        //

        // Treated Done.
        //cv::Mat test = *(outputFrames.begin());
        tools::drawDetections(frame, boxes, classIds, confidences);
        cv::namedWindow("YOLOv8 Detection (ONNX Runtime)", cv::WINDOW_NORMAL);
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {
        //    break;
        }
    //}
    //cv::destroyAllWindows();
    cv::waitKey(0);
    return 0;
}