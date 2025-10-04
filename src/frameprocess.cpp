#include "../include/frameprocess.hpp"
#include "../include/armor.hpp"
#include <vector>
#include "../include/tools.hpp"




cv::Mat frameProcess::cropRotatedRect(cv::Mat &frame, const cv::RotatedRect &rRect)
{
    float width = rRect.size.width;
    float height = rRect.size.height;
    float angle = rRect.angle;
    cv::Size2f rectSize(width, height);
    if (angle < -45.f)
    {
        angle += 90.0;
        std::swap(rectSize.width, rectSize.height);
    }
    else if (angle > 45.f)
    {
        angle -= 90.0;
        std::swap(rectSize.width, rectSize.height);
    }
    cv::Mat M = cv::getRotationMatrix2D(rRect.center, angle, 1.0);
    cv::Mat rotatedFrame;
    cv::warpAffine(frame, rotatedFrame, M, frame.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    cv::Rect bbox = rRect.boundingRect();
    cv::Point2f cropStart = bbox.tl();
    cv::Rect2f targetRect(0, 0, rectSize.width, rectSize.height);
    cv::Mat croppedImage;
    cv::getRectSubPix(rotatedFrame, rectSize, rRect.center, croppedImage);
    frame = croppedImage.clone();
    return croppedImage;
}


//Below function is old and shuldn't be used
//use yolov8 to detect instead.
//I am considering whether we need to have a OpenVino sample
//Anyway
cv::RotatedRect frameProcess::GetArmorRect(cv::Mat &image, const Lightbar_Pair &inputPairs)
{
    std::vector<cv::Point2f> contour_points;
    cv::RotatedRect a_ret = inputPairs.left_LightBar;
    cv::RotatedRect b_ret = inputPairs.right_LightBar;
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
    ap.x = a_ret.center.x + alon / 2.000000;
    bp.x = a_ret.center.x + alon / 2.000000;
    cp.x = b_ret.center.x - blon / 2.000000;
    dp.x = b_ret.center.x - blon / 2.000000;
    ap.y = a_ret.center.y - alen;
    bp.y = a_ret.center.y + alen;
    cp.y = b_ret.center.y - blen;
    dp.y = b_ret.center.y + blen;
    contour_points.push_back(ap);
    contour_points.push_back(bp);
    contour_points.push_back(cp);
    contour_points.push_back(dp);
    cv::RotatedRect rRect = cv::minAreaRect(contour_points);
    /*
    cv::circle(image, ap, 5, cv::Scalar(0, 255, 0), -1);
    cv::circle(image, bp, 5, cv::Scalar(0, 255, 0), -1);
    cv::circle(image, cp, 5, cv::Scalar(0, 255, 0), -1);
    cv::circle(image, dp, 5, cv::Scalar(0, 255, 0), -1);
    */
    // DrawRotatedRect(image,rRect,color,thickness);
    // cropRotatedRect(image,rRect);
    return rRect;
}

bool frameProcess::chk_vaild(cv::RotatedRect &rotrect)
{
    double ang = rotrect.angle;
    double wid = rotrect.size.width;
    double hei = rotrect.size.height;
    if (hei <= 7 && wid <= 7)
        return 0; // too small
    if (hei >= 100 || wid >= 100)
        return 0; // too big
    if (ang <= 0)
        ang = -ang;
    if (ang > 90)
        ang = 180 - ang;
    if (ang > 0 && ang <= 45)
    {
        ang = 90 - ang;
        double t = wid;
        wid = hei;
        hei = wid;
    }
    if (hei > 35)
        return 0;
    double rat = wid / hei;
    if (rat < 3.0 || rat > 15.0)
        return 0;
    return 1;
}

std::vector<cv::RotatedRect> frameProcess::findRect(const cv::Mat &inputFrame)
{
    cv::Mat gray_img;
    cv::cvtColor(inputFrame, gray_img, cv::COLOR_BGR2GRAY);
    cv::Mat binary_image;
    cv::threshold(gray_img, binary_image, 120, 255, cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<std::vector<cv::Point>> filteredContours;
    double minAreaThreshold = 50; // 设定最小面积阈值（根据你的图像调整）
    for (const auto& contour : contours) {
    // 1. 计算轮廓面积
        double area = cv::contourArea(contour);

        // 2. 过滤：如果面积大于阈值，则保留
        if (area > minAreaThreshold) {
            filteredContours.push_back(contour);
        }
    }
    std::vector<cv::RotatedRect> rotaterects;
    for(const auto & contour : filteredContours){
        auto rotaterect = cv::minAreaRect(contour);
        if (chk_vaild(rotaterect))
            rotaterects.emplace_back(rotaterect);
    }
    return rotaterects;
    // cv::drawContours(processedFrame, contours, -1 ,cv::Scalar(0,0,255),2);
}

std::vector<Lightbar_Pair> frameProcess::findPairs(std::vector<cv::RotatedRect> inputRects, const cv::Mat &inputFrame)
{
    std::vector<cv::RotatedRect> exersiRects;
    for (const auto &rects : inputRects)
    {
        exersiRects.emplace_back(rects);
    }
    std::vector<Lightbar_Pair> out_light;
    auto a = exersiRects.begin();
    auto b = exersiRects.begin();

    while (a != exersiRects.end())
    {
        if (((((*a).center.x) * ((*a).center.x)) + (((*a).center.y) * ((*a).center.y))) > 0)
        {
            auto b = exersiRects.begin();
            while (b != exersiRects.end())
            {
                if (((((*b).center.x) * ((*b).center.x)) + (((*b).center.y) * ((*b).center.y))) > 0)
                {
                    cv::Point2f apoint = (*a).center;
                    cv::Point2f bpoint = (*b).center;
                    double alen = ((*a).size.height > (*a).size.width ? (*a).size.height : (*a).size.width);
                    double blen = ((*b).size.height > (*b).size.width ? (*b).size.height : (*b).size.width);
                    double alon = ((*a).size.height < (*a).size.width ? (*a).size.height : (*a).size.width);
                    double blon = ((*b).size.height < (*b).size.width ? (*b).size.height : (*b).size.width);
                    double cmpx = (alen + blen) / 2.0000000;
                    double cmpy = (alon + blon) / 2.0000000;
                    double disx = apoint.x - bpoint.x;
                    double disy = apoint.y - bpoint.y;
                    if ((disx * disx) + (disy * disy) > 0)
                    {
                        /*
                        std::cout<<"Searching for these points"<<std::endl;
                        std::cout<<"A:"<<apoint.x<<" "<<apoint.y<<" "<<alen<<" "<<alon<<std::endl;
                        std::cout<<"B:"<<bpoint.x<<" "<<bpoint.y<<" "<<blen<<" "<<blon<<std::endl;
                        std::cout<<"Expected:"<<cmpx<<" "<<cmpy<<std::endl;
                        std::cout<<"Reality:"<<disx<<" "<<disy<<std::endl;
                        std::cout<<std::endl;
                        */
                        if (((disx < cmpx * 3.000000) && (disx > cmpx * -3.000000)) && ((disy < cmpy * 0.7500) && (disy > cmpy * -0.7500)))
                        {
                            Lightbar_Pair adds;
                            adds.left_LightBar = (*a);
                            adds.right_LightBar = (*b);
                            (*a).center.x = 0;
                            (*b).center.x = 0;
                            (*a).center.y = 0;
                            (*b).center.y = 0;
                            out_light.push_back(adds);
                            // std::cout<<"foundreal"<<std::endl;
                        }
                    }
                }
                b++;
            }
        }
        a++;
    }
    return out_light;
}

cv::Mat frameProcess::processFrame(const cv::Mat &inputFrame)
{
    cv::Mat processedFrame = inputFrame.clone();
    std::vector<cv::RotatedRect> rotaterects = findRect(inputFrame);
    // int stk = 0;

    // for(const auto & rectan : rotaterects){
    // std::cout<<rectan.center.x<<" "<<rectan.center.y<<std::endl;
    // DrawRotatedRect(processedFrame,rectan,cv::Scalar(0,255,0),2);
    // stk++;
    //}

    std::vector<Lightbar_Pair> lightbars = findPairs(rotaterects, inputFrame);

    for (const auto &ligpar : lightbars)
    {
        tools::DrawLightbars(processedFrame, ligpar, cv::Scalar(0, 255, 0), 2);
        cv::RotatedRect armor_re = GetArmorRect(processedFrame, ligpar);
        tools::DrawRotatedRect(processedFrame, armor_re, cv::Scalar(255, 255, 0), 2);
    }
    // std::cout<<std::endl<<lightbars.size()<<std::endl;
    return processedFrame;
}

cv::Mat frameProcess::processFrame_chopeed(const cv::Mat &inputFrame)
{
    cv::Mat processedFrame = inputFrame.clone();
    std::vector<cv::RotatedRect> rotaterects = findRect(inputFrame);
    // int stk = 0;
    /*
    for(const auto & rectan : rotaterects){
        //std::cout<<rectan.center.x<<" "<<rectan.center.y<<std::endl;
        //DrawRotatedRect(processedFrame,rectan,cv::Scalar(stk*50,255,stk*50),2);
        //stk++;
    }*/
    std::vector<Lightbar_Pair> lightbars = findPairs(rotaterects, inputFrame);
    for (const auto &ligpar : lightbars)
    {
        // DrawLightbars(processedFrame,ligpar,cv::Scalar(0,255,0),2);
        cv::RotatedRect armor_re = GetArmorRect(processedFrame, ligpar);
        // DrawRotatedRect(processedFrame,armor_re,cv::Scalar(255,255,0),2);
        cropRotatedRect(processedFrame, armor_re);
    }
    cv::Mat blank_frame(30, 30, CV_8UC3, cv::Scalar(0, 0, 0));
    if (processedFrame.rows == inputFrame.rows)
    {
        return blank_frame;
    }
    // std::cout<<std::endl<<lightbars.size()<<std::endl;
    return processedFrame;
}