#include "../include/failback.hpp"

//线性外推做法

std::vector<cv::Point2f> failbackFunc::GetArmorRect(cv::Mat &image, Lightbar_Pair &inputPairs)
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

    ap.x = a_ret.center.x + (alon / 2.000000) * acos + alen * 1.000000 * asin;
    bp.x = a_ret.center.x + (alon / 2.000000) * acos - alen * 1.000000 * asin;
    cp.x = b_ret.center.x - (blon / 2.000000) * bcos + blen * 1.000000 * bsin;
    dp.x = b_ret.center.x - (blon / 2.000000) * bcos - blen * 1.000000 * bsin;
    ap.y = a_ret.center.y + (alon / 2.000000) * asin - alen * 1.000000 * acos;
    bp.y = a_ret.center.y + (alon / 2.000000) * asin + alen * 1.000000 * acos;
    cp.y = b_ret.center.y - (blon / 2.000000) * bsin - blen * 1.000000 * bcos;
    dp.y = b_ret.center.y - (blon / 2.000000) * bsin + blen * 1.000000 * bcos;
    contour_points.push_back(ap);
    contour_points.push_back(bp);
    contour_points.push_back(cp);
    contour_points.push_back(dp);
    cv::RotatedRect rRect = cv::minAreaRect(contour_points);
    // tools::drawLightbars(image, inputPairs, cv::Scalar(0, 255, 0), 2);
    Lightbar_Pair outp;
    outp.left_LightBar = a_ret;
    outp.right_LightBar = b_ret;
    tools::drawLightbars(image, outp, cv::Scalar(0, 255, 0), 2);
    // cv::circle(image, ap, 10, cv::Scalar(0, 255, 255), -1);   // left up
    // cv::circle(image, bp, 10, cv::Scalar(255, 255, 0), -1);   // left down
    // cv::circle(image, cp, 10, cv::Scalar(255, 255, 255), -1); // right up
    // cv::circle(image, dp, 10, cv::Scalar(0, 255, 0), -1);     // right down
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

bool isApproximatelyParallel(const cv::RotatedRect& rect1,
                            const cv::RotatedRect& rect2,
                            double angleTolerance )//default 5 some condition 10
{
    double angle1 = rect1.angle;
    double angle2 = rect2.angle;

    double angleDiff = std::abs(angle1 - angle2);
    if (angleDiff <= angleTolerance) {
        return true;
    }
    double diff90 = std::abs(90.0 - angleDiff);
    if (diff90 <= angleTolerance) {
        return true;
    }

    return false;
}

std::vector<Lightbar_Pair> findPairs_old(std::vector<cv::RotatedRect> inputRects, const cv::Mat &inputFrame)
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

std::vector<Lightbar_Pair> failbackFunc::findPairs(std::vector<cv::RotatedRect> inputRects, const cv::Mat &inputFrame){
    return findPairs_old(inputRects,inputFrame);
}