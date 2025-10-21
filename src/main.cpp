#include "../include/include_all.hpp"
#include "../include/tools.hpp"
#include "../include/failback.hpp"
#include "../include/yolodetect.hpp"

int main(int argc, char **argv)
{
    std::vector<cv::Point3f> POINT_3D_OF_ARMOR_BIG = std::vector<cv::Point3f>
        {
            cv::Point3f(-105, -30, 0), // tl
            cv::Point3f(105, -30, 0), // tr
            cv::Point3f(105, 30, 0), // br
            cv::Point3f(-105, 30, 0) // bl
        };
    std::vector<cv::Point3f> POINT_3D_OF_ARMOR_SMALL = std::vector<cv::Point3f>
{
	cv::Point3f(-65, -35, 0),	//tl
	cv::Point3f(65, -35, 0),	//tr
	cv::Point3f(65, 35, 0),		//br
	cv::Point3f(-65, 35, 0)		//bl
};
    cv::Mat CAMERA_MATRIX = (cv::Mat_<double>(3, 3) <<
        1.0391876564768361e+03, 0, 9.4661629904337065e+02,
        0, 1.0349035035562770e+03, 5.3417040085263443e+02,
        0, 0, 1
    );
    DigitTemplates templ = MatchNumber::loadTemplates("templates/", 1);
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
            //    tools::drawLightbars(frame, armorA.Lightbars, cv::Scalar(0, 255, 0), 1);
        }
        // Otherwise ERROR POST.
        std::vector<Armor> armorFiltered;
        if (armors.size() > 0)
        {
            // PartA: DetectNumbers
            // StepA: Cut & Check & DetectNumbers
            for (int i = 0; i < armors.size(); ++i)
            {
                cv::Mat armorFrame;
                armors[i].position = failbackFunc::GetArmorRect(frame, armors[i].Lightbars);
                if (tools::cropQuadrilateral(armors[i].position, frame, armorFrame))
                {
                    // 这个地方 以后使用CUDA优化
                    MatchResult resultas = MatchNumber::recognizeSingleDigitByFeature(armorFrame, templ);
                    // std::cout<<"Finish one"<<std::endl;
                    if (resultas.digit == -1 || resultas.score < 0.5)
                    {
                        continue;
                    }
                    else
                    {
                        Armor finalOne = armors[i];
                        finalOne.car_num = resultas.digit;
                        armorFiltered.push_back(finalOne);
                    }
                }
                cv::Mat gray, binary;
                cv::cvtColor(armorFrame, gray, cv::COLOR_BGR2GRAY);
                cv::threshold(gray, binary, 53, 255, cv::THRESH_BINARY);
                cv::imshow("Test" + std::to_string(i), binary);
            }

            // PnP_Calculations
        }

        // Calculation for FPS Rate.
#ifdef VIDEO
        double t1 = cv::getTickCount();
        double time_per_frame = (t1 - t_last) / freq;
        fps = 1.00000 / time_per_frame;
        std::string fps_text = "FPS: " + std::to_string(static_cast<double>(fps));
        cv::putText(frame,
                    fps_text,
                    cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX,
                    1,
                    cv::Scalar(0, 255, 0),
                    2);
        t_last = t1;
        cv::imshow("Output", frame);
#endif

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