
#include "../include/failback.hpp"

// 线性外推做法

std::vector<cv::Point2f> failbackFunc::GetArmorRect(cv::Mat &image, Lightbar_Pair &inputPairs, int mode)
{
    std::vector<cv::Point2f> contour_points;
    cv::RotatedRect a_ret = tools::getNormalizedRotatedRect_fortyfive(inputPairs.left_LightBar);
    cv::RotatedRect b_ret = tools::getNormalizedRotatedRect_fortyfive(inputPairs.right_LightBar);
    if (b_ret.center.x < a_ret.center.x)
    {
        cv::RotatedRect c_ret = a_ret;
        a_ret = b_ret;
        b_ret = c_ret;
    }
    double alen = (a_ret.size.height > a_ret.size.width ? a_ret.size.height : a_ret.size.width);
    double blen = (b_ret.size.height > b_ret.size.width ? b_ret.size.height : b_ret.size.width);
    double alon = (a_ret.size.height < a_ret.size.width ? a_ret.size.height : a_ret.size.width);
    double blon = (b_ret.size.height < b_ret.size.width ? b_ret.size.height : b_ret.size.width);

    cv::Point2f ap, bp, cp, dp;

    // std::cout << a_ret.angle << ' ' << b_ret.angle << std::endl;
    double asin = sin((a_ret.angle) / 360.000000 * pi), bsin = sin((b_ret.angle) / 360.000000 * pi),
           acos = cos((a_ret.angle) / 360.000000 * pi), bcos = cos((b_ret.angle) / 360.000000 * pi);
    // std::cout << "a:angle:" << a_ret.angle << ' ' << (a_ret.angle) / 360.00000 * pi << ' ' << asin << ' ' << acos << std::endl;
    // std::cout << "b:angle:" << a_ret.angle << ' ' << (b_ret.angle) / 360.00000 * pi << ' ' << bsin << ' ' << bcos << std::endl;

    ap.x = a_ret.center.x + (-mode)*(alon / 2.000000) * acos + alen * 1.000000 * asin;
    bp.x = a_ret.center.x + (-mode)*(alon / 2.000000) * acos - alen * 1.000000 * asin;
    cp.x = b_ret.center.x - (-mode)*(blon / 2.000000) * bcos + blen * 1.000000 * bsin;
    dp.x = b_ret.center.x - (-mode)*(blon / 2.000000) * bcos - blen * 1.000000 * bsin;
    ap.y = a_ret.center.y + (-mode)*(alon / 2.000000) * asin - alen * 1.000000 * acos;
    bp.y = a_ret.center.y + (-mode)*(alon / 2.000000) * asin + alen * 1.000000 * acos;
    cp.y = b_ret.center.y - (-mode)*(blon / 2.000000) * bsin - blen * 1.000000 * bcos;
    dp.y = b_ret.center.y - (-mode)*(blon / 2.000000) * bsin + blen * 1.000000 * bcos;
    contour_points.push_back(ap);
    contour_points.push_back(cp);
    contour_points.push_back(dp);
    contour_points.push_back(bp);
    cv::RotatedRect rRect = cv::minAreaRect(contour_points);
    // tools::drawLightbars(image, inputPairs, cv::Scalar(0, 255, 0), 2);
    Lightbar_Pair outp;
    outp.left_LightBar = a_ret;
    outp.right_LightBar = b_ret;
    // tools::drawLightbars(image, outp, cv::Scalar(0, 255, 0), 1);
    /*
      cv::circle(image, ap, 10, cv::Scalar(0, 255, 255), -1);   // left up
      cv::circle(image, bp, 10, cv::Scalar(255, 255, 0), -1);   // left down
      cv::circle(image, cp, 10, cv::Scalar(255, 255, 255), -1); // right up
      cv::circle(image, dp, 10, cv::Scalar(0, 255, 0), -1);     // right down
    */
    return contour_points;
}

bool failbackFunc::checkVaild(cv::RotatedRect inputRect)
{
    if (inputRect.size.area() < 50)
        return 0;
    if (inputRect.size.height < inputRect.size.width * 3.000000)
        return 0;
    if (inputRect.size.height > inputRect.size.width * 15.000000)
        return 0;
    return 1;
}

bool isApproximatelyParallel(const cv::RotatedRect &rect1,
                             const cv::RotatedRect &rect2,
                             double angleTolerance) // default 5 some condition 10
{
    double angle1 = rect1.angle;
    double angle2 = rect2.angle;

    double angleDiff = std::abs(angle1 - angle2);
    if (angleDiff <= angleTolerance)
    {
        return true;
    }
    double diff90 = std::abs(90.0 - angleDiff);
    if (diff90 <= angleTolerance)
    {
        return true;
    }

    return false;
}

std::vector<Lightbar_Pair> failbackFunc::findPairs(std::vector<cv::RotatedRect> inputRects, const cv::Mat &inputFrame)
{
    std::vector<Lightbar_Pair> lightBars;

    std::vector<int> canitbe;
    for (int i = 0; i < inputRects.size(); ++i)
    {
        canitbe.push_back(0);
    }

    for (int i = 0; i < inputRects.size(); ++i)
    {
        for (int j = 0; j < inputRects.size(); ++j)
        {
            if (((!(canitbe[i])) && (!(canitbe[j]))) && (i != j))
            {
                // 需要一个条件判断
                auto a = inputRects[i];
                auto b = inputRects[j];
                auto a_x = a.center.x;
                auto a_y = a.center.y;
                auto b_x = b.center.x;
                auto b_y = b.center.y;
                auto center_dis = sqrt((a_x - b_x) * (a_x - b_x) + (a_y - b_y) * (a_y - b_y));
                auto minus_corr = abs(abs(a.angle) - abs(b.angle));
                auto averageHeight = (a.size.height + b.size.height) / 2.000000;
                auto averageWidth = (a.size.width + b.size.width) / 2.000000;
                if (((center_dis < averageHeight * 3.0000000) && (minus_corr < 22) // distance check
                     && abs(abs(atan(abs(a_y - b_y) / abs(a_x - b_x)) * 180 / pi) - minus_corr) < 3))
                { // position check
                    Lightbar_Pair ligs;
                    if (a_x > b_x)
                    {
                        ligs.left_LightBar = b;
                        ligs.right_LightBar = a;
                    }
                    else
                    {
                        ligs.left_LightBar = a;
                        ligs.right_LightBar = b;
                    }
                    double rate = center_dis * 1.000000 / (averageHeight * 2.000000);
                    if(rate < 1.90000){
                        ligs.size = 0;
                    }else ligs.size = 1;
                    lightBars.push_back(ligs);
                    canitbe[i] = 1;
                    canitbe[j] = 1;
                }
            }
        }
    }
    return lightBars;
}

bool failbackFunc::checkEnemy(cv::RotatedRect lightbar, cv::Mat frame)
{
    Lightbar_Pair a;
    a.left_LightBar = lightbar;
    a.right_LightBar = lightbar;

    auto raw_rect = tools::bounding_rect_of_dual_rotated_rects(a);
    cv::Rect image_bounds(0, 0, frame.cols, frame.rows);

    cv::Rect safe_rect = raw_rect & image_bounds;

    if (safe_rect.empty())
    {
        return 0;
    }

    cv::Mat lightbarMat = frame(safe_rect);

    std::vector<cv::Mat> channels;
    cv::split(lightbarMat, channels);

    cv::Mat checkvaildFrame;
    if (channels.size() < 3)
        return 0; // 安全检查

    cv::Mat frame2 = channels[0] - channels[2];
    cv::threshold(frame2, checkvaildFrame, 130, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point>> contoursbackup;
    cv::findContours(checkvaildFrame, contoursbackup, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    if (contoursbackup.size() > 0)
        return 0;

    return 1;
}

std::vector<Armor> failbackFunc::mainFunction(cv::Mat frame)
{
    std::vector<Armor> armorsBackup;
    std::vector<cv::Mat> channelsBackup;
    cv::split(frame, channelsBackup);
    cv::Mat binaryImageBack;
    cv::Mat preImageBack = channelsBackup[_ENEMY].clone();
    cv::threshold(preImageBack, binaryImageBack, 130, 255, cv::THRESH_BINARY);
    // cv::imshow("Binary Test", binaryImageBack);
    std::vector<std::vector<cv::Point>> contoursbackup;
    cv::findContours(binaryImageBack, contoursbackup, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<cv::RotatedRect> rotaterectsBackup;
    for (const auto &contour : contoursbackup)
    {
        auto rotaterect = cv::minAreaRect(contour);
        auto finrect = tools::getNormalizedRotatedRect_fortyfive(rotaterect);
        // 这里需要一个敌我识别 现在的方法很容易不认自己人
        // if (failbackFunc::checkEnemy(finrect,frame))
        if (failbackFunc::checkEnemy(finrect, frame))
        {
            // std::cout<<1<<std::endl;

            if (failbackFunc::checkVaild(finrect))
            {
                rotaterectsBackup.emplace_back(finrect);
                // tools::drawRotatedRect(frame, finrect, cv::Scalar(0, 255, 0), 2);
            }
        }
    }
    // cv::drawContours(frame, contoursbackup, -1 ,cv::Scalar(0,255,0),2);
    // failBack::findPairs

    std::vector<Lightbar_Pair> outLightBarBack = failbackFunc::findPairs(rotaterectsBackup, frame);
    for (const auto &lightbar : outLightBarBack)
    {
        // tools::drawLightbars(frame, lightbar, cv::Scalar(0, 255, 0), 2);
        Armor lar;
        lar.Lightbars = lightbar;
        armorsBackup.push_back(lar);
    }
    return armorsBackup;
}