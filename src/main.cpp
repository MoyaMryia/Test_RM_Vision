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
        frame = tools::adjustBrightness(tools::enhanceContrast(frame),5);
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
        std::vector<cv::Mat> outputFrames_pre;
        std::vector<cv::Mat> outputFrames;
        outputFrames_pre = tools::chopFrame(armors, frame);

        // 抠数字
        // contours里面包含一个抠出来的数字
        // RotateRect也有
        // 之后会转移到后处理函数里
        
        if (outputFrames_pre.size() > 0)
        {
            for (auto &frame_try2 : outputFrames_pre)
            {
                cv::Mat frame_try;
                frame_try = frame_try2;//tools::enhanceContrast(tools::adjustBrightness(frame_try2,15));
                std::vector<std::vector<cv::Point>> contours = tools::getContours(frame_try,0);
                //Obviously an Armor only consist 2
                cv::Mat frame_out = frame_try.clone();
                std::vector<cv::RotatedRect> rotaterects;
                for (const auto &contour : contours)
                {
                    auto rotaterect = cv::minAreaRect(contour);
                    rotaterects.emplace_back(rotaterect);
                    
                }
                std::sort(rotaterects.begin(), rotaterects.end(), 
                    [](const cv::RotatedRect& a, const cv::RotatedRect& b) {
                        return a.size.area() > b.size.area();
                    });
                tools::drawRotatedRect(frame_out,rotaterects[0],cv::Scalar(0,255,0),1);
                tools::drawRotatedRect(frame_out,rotaterects[1],cv::Scalar(0,255,0),1);
                //tools::drawRotatedRect(frame_out,rotaterect,cv::Scalar(0,255,0),2);
                //cv::drawContours(frame_out, contours, -1 ,cv::Scalar(0,255,0),2);
                outputFrames.push_back(frame_out);
            }
        }
        //类似的方法 写一个抠光条的



        // There should be something that can be treated;
        // A. afterDetections::mainCalculations
        // B. create a std::vector<Robot> and take the armors into it;
        // C. Kalman Calculations
        // D. And finally, get a mark, done.

        if (outputFrames.size() > 0)
        {
            // Below Are Test codes.
            cv::Mat test = *(outputFrames.begin());
            cv::Mat test2 = *(outputFrames_pre.begin());
            cv::imshow("Chopped Colored", test);
            //cv::imshow("Chopped", test2);
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