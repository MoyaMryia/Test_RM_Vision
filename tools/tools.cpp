#include "../include/tools.hpp"
#include <opencv2/opencv.hpp>
float maxx(float a, float b)
{
    if (a > b)
        return a;
    else
        return b;
}
float minn(float a, float b)
{
    if (a < b)
        return a;
    else
        return b;
}
const std::vector<std::string> CLASS_NAMES = {
    "armor_blue", "armor_grey", "armor_red",
    "car_blue", "car_red", "car_unknown",
    "watcher_blue", "watcher_red", "watcher_unknown"};

// 测试筛选的时候使用的 现在我估计用处不大
cv::Mat tools::enhanceContrast(const cv::Mat &inputFrame)
{
    if (inputFrame.empty())
    {
        std::cerr << "Error: Input frame is empty." << std::endl;
        return cv::Mat();
    }

    cv::Mat outputFrame;

    if (inputFrame.channels() == 3)
    {
        // 对于彩色图像，直接对 RGB 通道进行均衡化可能会导致颜色失真。
        // 最佳做法是将其转换到 YUV 或 HSV 颜色空间，只对亮度/明度通道 (Y 或 V) 进行均衡化。

        cv::Mat yuv;
        // 转换到 YUV 颜色空间 (Y 是亮度通道)
        cv::cvtColor(inputFrame, yuv, cv::COLOR_BGR2YUV);

        // 分割 YUV 图像到三个通道
        std::vector<cv::Mat> channels;
        cv::split(yuv, channels); // channels[0] 是 Y (亮度) 通道

        // 对亮度通道进行直方图均衡化
        // 示例：使用 CLAHE 代替基本的 equalizeHist
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(1.5, cv::Size(8, 8));
        // 2.0 是对比度限制，(8, 8) 是分块大小
        clahe->apply(channels[0], channels[0]); // 应用于亮度通道

        // 合并通道回 YUV 图像
        cv::merge(channels, yuv);

        // 转换回 BGR 颜色空间作为输出
        cv::cvtColor(yuv, outputFrame, cv::COLOR_YUV2BGR);
    }
    // 3. 处理灰度图像 (1 通道)
    else if (inputFrame.channels() == 1)
    {
        // 对于灰度图像，直接应用直方图均衡化
        cv::equalizeHist(inputFrame, outputFrame);
    }
    else
    {
        std::cerr << "Warning: Unsupported number of channels. Returning original image." << std::endl;
        outputFrame = inputFrame.clone();
    }

    return outputFrame;
}

cv::Mat tools::adjustBrightness(const cv::Mat &inputFrame, double beta)
{

    if (inputFrame.empty())
    {
        std::cerr << "Error: Input frame is empty." << std::endl;
        return cv::Mat();
    }

    cv::Mat outputFrame;
    // 保持 alpha 为 1.0，只调整亮度
    double alpha = 1.0;

    // outputFrame = alpha * inputFrame + beta
    // 确保 beta 是整数。
    inputFrame.convertTo(outputFrame, -1, alpha, (double)beta);

    return outputFrame;
}

std::vector<cv::Mat> tools::chopFrame(const std::vector<Armor> &inputArmors, const cv::Mat &inputFrame)
{
    std::vector<cv::Mat> outputFrame;
    for (const auto &armor : inputArmors)
    {
        cv::Rect choppingRect = armor.Box;
        outputFrame.push_back(inputFrame(choppingRect));
    }

    return outputFrame;
}


void tools::drawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness)
{
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);
    for (int i = 0; i < 4; i++)
    {
        line(image, vertices[i], vertices[(i + 1) % 4], color, thickness);
    }
}

void tools::drawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness)
{
    tools::drawRotatedRect(image, inputPairs.left_LightBar, color, thickness);
    tools::drawRotatedRect(image, inputPairs.right_LightBar, color, thickness);
}

std::vector<std::vector<cv::Point>> tools::getContours(cv::Mat &inputFrame, double minAreaThreshold)
{
    cv::Mat gray_img;
    cv::cvtColor(inputFrame, gray_img, cv::COLOR_BGR2GRAY);
    cv::Mat binary_image;

    cv::threshold(gray_img, binary_image, 120, 255, cv::THRESH_BINARY);
    // cv::imshow("binary",binary_image);
    // cv::imshow("grey",gray_img);
    // get contours

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<std::vector<cv::Point>> filteredContours;

    for (const auto &contour : contours)
    {
        double area = cv::contourArea(contour);
        if (area > minAreaThreshold)
        {
            filteredContours.push_back(contour);
        }
    }
    // cv::drawContours(inputFrame, filteredContours, -1, cv::Scalar(0, 0, 255), 2);
    return filteredContours;
}

void tools::classifyArmors(long long &total, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences, std::vector<Robot> &cars, std::vector<Robot> &watchers, std::vector<Armor> &armors)
{
    for (size_t i = 0; i < boxes.size(); ++i)
    {
        if (classIds[i] < 3)
        {
            // 这里先筛选所有装甲
            // blue 0 white 1 red 2
            Armor t;
            t.Box = boxes[i];
            t.confidence = confidences[i];
            t.color = cv::Scalar(255 * minn(1, (classIds[i] + 1) % 3), 255 * ((classIds[i]) % 2), minn(255, classIds[i] * 255)); // Actually, this is trickey!
            t.detect_id = total;
            total++;
            t.classId = classIds[i];
            armors.push_back(t);
        }
        else
        {
            Robot t;
            t.robotrect = boxes[i];
            t.confidence = confidences[i];
            if (classIds[i] < 6)
            {
                int k = classIds[i] - 3;
                // blue 0 red 1 white 2;
                t.color = cv::Scalar(maxx(255, std::abs(k - 1) * 255), maxx(0, 255 * (k - 1)), min(255, 255 * k));
                t.carorwatcher = 1;
                cars.push_back(t);
            }
            else
            {
                int k = classIds[i] - 6;
                // blue 0 red 1 white 2;
                t.color = cv::Scalar(maxx(255, std::abs(k - 1) * 255), maxx(0, 255 * (k - 1)), min(255, 255 * k));
                t.carorwatcher = 0;
                watchers.push_back(t);
            }
        }
    }
}
// Test Functions Don't Use in FINAL.

void tools::drawDetections(cv::Mat &img, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences)
{

    for (size_t i = 0; i < boxes.size(); ++i)
    {
        cv::rectangle(img, boxes[i], cv::Scalar(0, 255, 0), 2);
        std::string label = CLASS_NAMES[classIds[i]] + cv::format(": %.2f", confidences[i]);

        int baseLine;
        cv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int top = boxes[i].tl().y;
        cv::rectangle(img, cv::Point(boxes[i].tl().x, top - label_size.height - baseLine),
                      cv::Point(boxes[i].tl().x + label_size.width, top),
                      cv::Scalar(0, 255, 0), cv::FILLED);

        cv::putText(img, label, cv::Point(boxes[i].tl().x, top - baseLine),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
}

cv::RotatedRect tools::getNormalizedRotatedRect_fortyfive(const cv::RotatedRect &rect)
{
    cv::RotatedRect normalizedRect = rect;

    float angle = normalizedRect.angle;
    float width = normalizedRect.size.width;
    float height = normalizedRect.size.height;

    if (width < height)
    {
        std::swap(width, height);
        angle += 90.0f;
    }

    if (angle >= 90.0f)
    {
        angle -= 180.0f;
    }
    else if (angle < -90.0f)
    {
        angle += 180.0f;
    }

    if (angle > 45.0f)
    {
        angle -= 90.0f;
        std::swap(width, height); 
    }
    else if (angle <= -45.0f)
    {
        angle += 90.0f;
        std::swap(width, height); 
    }

    normalizedRect.angle = angle;
    normalizedRect.size.width = width;
    normalizedRect.size.height = height;

    return normalizedRect;
}

cv::Rect tools::bounding_rect_of_dual_rotated_rects(const Lightbar_Pair& dualRects)
{
    cv::Point2f vertices1[4];
    dualRects.left_LightBar.points(vertices1);
    cv::Point2f vertices2[4];
    dualRects.right_LightBar.points(vertices2);

    float min_x = vertices1[0].x;
    float max_x = vertices1[0].x;
    float min_y = vertices1[0].y;
    float max_y = vertices1[0].y;

    for (int i = 0; i < 4; i++)
    {
        min_x = std::min(min_x, vertices1[i].x);
        max_x = std::max(max_x, vertices1[i].x);
        min_y = std::min(min_y, vertices1[i].y);
        max_y = std::max(max_y, vertices1[i].y);
    }
    for (int i = 0; i < 4; i++)
    {
        min_x = std::min(min_x, vertices2[i].x);
        max_x = std::max(max_x, vertices2[i].x);
        min_y = std::min(min_y, vertices2[i].y);
        max_y = std::max(max_y, vertices2[i].y);
    }
    
    int x = static_cast<int>(std::floor(min_x));
    int y = static_cast<int>(std::floor(min_y));
    
    int width = static_cast<int>(std::ceil(max_x)) - x;
    int height = static_cast<int>(std::ceil(max_y)) - y;

    width = std::max(1, width);
    height = std::max(1, height);
    
    return cv::Rect(x, y, width, height);
}

bool tools::is_pair_approx_equal(const Lightbar_Pair& pair1, const Lightbar_Pair& pair2){
    cv::Rect a_ret = bounding_rect_of_dual_rotated_rects(pair1),b_ret = bounding_rect_of_dual_rotated_rects(pair2);
    cv::Rect t = a_ret & b_ret;
    if(t.area()*1.000000/(((a_ret.area()+b_ret.area())*1.000000)/2) > 0.7) return 1;
    return 0;
}