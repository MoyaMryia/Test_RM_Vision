#include "../include/include_all.hpp"

int main(int argc, char **argv)
{
#ifdef USING_YOLO
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH))
    {
        return -1;
    }
#endif
    VideoReader reader(argv[1]);
    if (!reader.isOpened())
    {
        return -1;
    }
    cv::Mat frame;
    long long total = 1;
    double t0 = 0;
    double t_last = cv::getTickCount();
    double freq = cv::getTickFrequency();
    double fps = 0.0;
    while (true)
    {
        t0 = cv::getTickCount();
        if (!reader.readFrame(frame))
        {
            break;
        }
#ifdef USING_YOLO
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

        // 最后优化再拆分代码得了

        std::vector<Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);
        std::vector<Armor> armorsYolo;
        std::vector<Robot> cars;
        std::vector<Robot> watchers;
        tools::classifyArmors(total, boxes, classIds, confidences, cars, watchers, armorsYolo);
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
                    armorsYolo[i].Lightbars.left_LightBar = rotaterects[0];
                    armorsYolo[i].Lightbars.right_LightBar = rotaterects[1];
                    armorsYolo[i].position = failbackFunc::GetArmorRect(outputFrames[i], armorsYolo[i].Lightbars);
                    // tools::drawLightbars(outputFrames[i],armorsYolo[i].Lightbars,cv::Scalar(0,255,0),2);
                }
                // tools::drawDetections(frame, boxes, classIds, confidences);
                cv::imshow("output_binary" + std::to_string(i), binaryImage);
                cv::imshow("output_pre" + std::to_string(i), preImage);
            }
        }
#endif
#ifdef USING_BACKUP
        std::vector<Armor> armorsBackup;
#endif


        // Merge these things
        std::vector<Armor> armors;
#if defined(USING_YOLO) && defined(USING_BACKUP)

        armors.reserve(armorsBackup.size() + armorsYolo.size());
        armors = armorsYolo;
        for (const auto &armorCons : armorsBackup)
        {
            bool is_duplicate = false;
            is_duplicate = std::any_of(armors.begin(), armors.end(),
                                       [&armorCons](const Armor &existing_armor)
                                       {
                                           return tools::is_pair_approx_equal(armorCons.Lightbars, existing_armor.Lightbars) == 1;
                                       });
            if (!is_duplicate)
            {
                armors.push_back(armorCons);
            }
        }

#endif
// Without anyone
#if !defined(USING_YOLO) && defined(USING_BACKUP)
        armors = armorsBackup;
#endif
#if defined(USING_YOLO) && !defined(USING_BACKUP)
        armors = armorsYolo;
#endif
//Otherwise ERROR POST.
//Calculation for FPS Rate.
        double t1 = cv::getTickCount();
        double time_per_frame = (t1 - t_last) / freq;
        fps = 1.0 / time_per_frame;
        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));
        cv::putText(frame,
                    fps_text,
                    cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX,
                    1,
                    cv::Scalar(0, 255, 0),
                    2);
        t_last = t1;
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}