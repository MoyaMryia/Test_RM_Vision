#include "../include/failback.hpp"

cv::RotatedRect failbackFunc::GetArmorRect(cv::Mat &image, const Lightbar_Pair &inputPairs)
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