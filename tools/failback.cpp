#include "../include/failback.hpp"

cv::RotatedRect failbackFunc::getNormalizedRotatedRect_fortyfive(const cv::RotatedRect &rect)
{
    // 复制原始矩形，在新对象上进行修改
    cv::RotatedRect normalizedRect = rect;

    float angle = normalizedRect.angle;
    float width = normalizedRect.size.width;
    float height = normalizedRect.size.height;

    // ----------------------------------------------------------------------
    // 步骤 1: 预标准化到 [-90, 90) 范围 (确保 width 暂为长边)
    // ----------------------------------------------------------------------
    if (width < height)
    {
        // 确保 width >= height
        std::swap(width, height);
        angle += 90.0f;
    }

    // 调整到 [-90, 90)
    if (angle >= 90.0f)
    {
        angle -= 180.0f;
    }
    else if (angle < -90.0f)
    {
        angle += 180.0f;
    }

    // 此时 angle 在 [-90, 90)，width 是长边。

    // ----------------------------------------------------------------------
    // 步骤 2: 统一到 [-45, 45) 范围 (如果需要，交换 width/height)
    // ----------------------------------------------------------------------

    if (angle > 45.0f)
    {
        // 角度在 (45, 90) 范围。例如：60度。
        // 新角度: 60 - 90 = -30度
        angle -= 90.0f;
        std::swap(width, height); // 交换 W/H，短边成为新的 width
    }
    else if (angle <= -45.0f)
    {
        // 角度在 [-90, -45] 范围。例如：-60度。
        // 新角度: -60 + 90 = 30度
        angle += 90.0f;
        std::swap(width, height); // 交换 W/H，短边成为新的 width
    }

    // 此时，angle 在 [-45, 45) 范围。
    // width 是旋转后更接近水平的边 (与 x 轴夹角绝对值 <= 45)。

    // 3. 更新并返回新的 RotatedRect
    normalizedRect.angle = angle;
    normalizedRect.size.width = width;
    normalizedRect.size.height = height;

    return normalizedRect;
}

std::vector<cv::Point2f> failbackFunc::GetArmorRect(cv::Mat &image, Lightbar_Pair &inputPairs)
{
    std::vector<cv::Point2f> contour_points;
    cv::RotatedRect a_ret = getNormalizedRotatedRect_fortyfive(inputPairs.left_LightBar);
    cv::RotatedRect b_ret = getNormalizedRotatedRect_fortyfive(inputPairs.right_LightBar);
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

    std::cout << a_ret.angle << ' ' << b_ret.angle << std::endl;
    double asin = sin((a_ret.angle) / 360.000000 * pi), bsin = sin((b_ret.angle) / 360.000000 * pi),
           acos = cos((a_ret.angle) / 360.000000 * pi), bcos = cos((b_ret.angle) / 360.000000 * pi);
    std::cout << "a:angle:" << a_ret.angle << ' ' << (a_ret.angle) / 360.00000 * pi << ' ' << asin << ' ' << acos << std::endl;
    std::cout << "b:angle:" << a_ret.angle << ' ' << (b_ret.angle) / 360.00000 * pi << ' ' << bsin << ' ' << bcos << std::endl;

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
    cv::circle(image, ap, 10, cv::Scalar(0, 255, 255), -1);   // left up
    cv::circle(image, bp, 10, cv::Scalar(255, 255, 0), -1);   // left down
    cv::circle(image, cp, 10, cv::Scalar(255, 255, 255), -1); // right up
    cv::circle(image, dp, 10, cv::Scalar(0, 255, 0), -1);     // right down
    return contour_points;
} /*  ap.x = a_ret.center.x + alen / 2.000000;
     bp.x = a_ret.center.x + alen / 2.000000;
     cp.x = b_ret.center.x - blen / 2.000000;
     dp.x = b_ret.center.x - blen / 2.000000;
     ap.y = a_ret.center.y - alen;
     bp.y = a_ret.center.y + alen;
     cp.y = b_ret.center.y - blen;
     dp.y = b_ret.center.y + blen;*/