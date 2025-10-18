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
#ifdef VIDEO
    long long total = 1;
    double t0 = 0;
    double t_last = cv::getTickCount();
    double freq = cv::getTickFrequency();
    double fps = 0.0;

    while (true)
    {

        t0 = cv::getTickCount();
#endif
        if (!reader.readFrame(frame))
        {
#ifdef VIDEO
            break;
#else
            return 0;
#endif
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
                // cv::imshow("output_binary" + std::to_string(i), binaryImage);
                // cv::imshow("output_pre" + std::to_string(i), preImage);
            }
        }
#endif

#ifdef USING_BACKUP
        std::cout<<0<<std::endl;
        std::vector<Armor> armorsBackup;
        // 有一个公共方法: tools::GetArmorRect
        std::vector<cv::Mat> channelsBackup;
        cv::split(frame, channelsBackup);
        cv::Mat binaryImageBack;
        cv::Mat preImageBack = channelsBackup[_ENEMY].clone();
        cv::threshold(preImageBack, binaryImageBack, 130, 255, cv::THRESH_BINARY);
        std::cout<<1<<std::endl;
        //cv::imshow("Binary Test", binaryImageBack);
        std::vector<std::vector<cv::Point>> contoursbackup;
        cv::findContours(binaryImageBack, contoursbackup, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        std::vector<cv::RotatedRect> rotaterectsBackup;
        std::cout<<2<<std::endl;
        for (const auto &contour : contoursbackup)
        {
            auto rotaterect = cv::minAreaRect(contour);
            auto finrect = tools::getNormalizedRotatedRect_fortyfive(rotaterect);
            if (failbackFunc::checkVaild(finrect))
            {
                rotaterectsBackup.emplace_back(finrect);
                // tools::drawRotatedRect(frame, finrect, cv::Scalar(0, 255, 0), 2);
            }
        }
        std::cout<<3<<std::endl;
        // cv::drawContours(frame, contoursbackup, -1 ,cv::Scalar(0,255,0),2);
        // failBack::findPairs
        std::vector outLightBarBack = failbackFunc::findPairs(rotaterectsBackup, frame);
        std::cout<<4<<std::endl;
        for (const auto &lightbar : outLightBarBack)
        {
            tools::drawLightbars(frame, lightbar, cv::Scalar(0, 255, 0), 2);
        }
        std::cout<<5<<std::endl;
#endif
        std::vector<Armor> armors;
// 这个Merge先不用 需要一些测试
#if defined(EXPERIMENTIAL)
        // Merge these things

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
#endif
        // Without anyone

#if !defined(USING_YOLO) && defined(USING_BACKUP)
        armors = armorsBackup;
#endif
#if defined(USING_YOLO) && !defined(USING_BACKUP)
        armors = armorsYolo;
#endif

        // Otherwise ERROR POST.
        // Kalman
        if(armors.size()>0){
            //PartA: DetectNumbers
            //StepA: Cut
            //StepB: Compare
            
            //StepC: Using the number to find the Rect.

            //PartB: Track

            //PartC: Track speed and yaws

        }
        // Calculation for FPS Rate.
#ifdef VIDEO
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
#endif
        cv::imshow("Original", frame);
#ifdef VIDEO
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27)
        {

            break;

        }
    }
#endif
#ifdef VIDEO
    cv::destroyAllWindows();
#else
    cv::waitKey(0);
#endif
    return 0;
}