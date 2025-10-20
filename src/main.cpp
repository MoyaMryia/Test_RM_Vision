#include "../include/include_all.hpp"
#include "../include/tools.hpp"
#include "../include/failback.hpp"
#include "../include/yolodetect.hpp"

int main(int argc, char **argv)
{
#ifdef USING_YOLO
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH))
    {
        return -1;
    }
    long long total = 1;
#endif
    VideoReader reader(argv[1]);
    if (!reader.isOpened())
    {
        return -1;
    }
    cv::Mat frame;
#ifdef VIDEO

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
        std::vector<Armor> armorsYolo = YOLOv8Detector::mainFunction(frame, detector, total);

#endif

#ifdef USING_BACKUP
        // 复杂情况准度低 但是常时有能力
        std::vector<Armor> armorsBackup = failbackFunc::mainFunction(frame);
        // 有一个公共方法: tools::GetArmorRect
#endif

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
#if !defined(USING_YOLO) && defined(USING_BACKUP)
        armors = armorsBackup;
#endif
#if defined(USING_YOLO) && !defined(USING_BACKUP)
        armors = armorsYolo;
#endif
        for (const auto &armorA : armors)
        {
            tools::drawLightbars(frame, armorA.Lightbars, cv::Scalar(0, 255, 0), 1);
        }
        // Otherwise ERROR POST.
        // Kalman
        if (armors.size() > 0)
        {
            // PartA: DetectNumbers
            // StepA: Cut
            for (int i = 0; i < armors.size(); ++i)
            {
                armors[i].position = failbackFunc::GetArmorRect(frame, armors[i].Lightbars);
            }
            // Step (A+B)/2 check if it's right
            // StepB: Compare

            // StepC: Using the number to find the Rect.

            // PartB: Track

            // PartC: Track speed and yaws
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