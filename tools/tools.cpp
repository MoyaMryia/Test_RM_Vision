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

cv::Mat tools::enhanceContrast(const cv::Mat &inputFrame)
{
    // 1. 检查输入图像是否为空
    if (inputFrame.empty())
    {
        std::cerr << "Error: Input frame is empty." << std::endl;
        return cv::Mat();
    }

    cv::Mat outputFrame;

    // 2. 处理彩色图像 (3 通道)
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
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
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
    // 4. 处理其他情况 (例如 4 通道，虽然不常见)
    else
    {
        // 如果输入不是 1 或 3 通道，则返回原图并警告
        std::cerr << "Warning: Unsupported number of channels. Returning original image." << std::endl;
        outputFrame = inputFrame.clone();
    }

    return outputFrame;
}

cv::Mat tools::adjustBrightness(const cv::Mat &inputFrame, int beta)
{
    // 1. 检查输入图像
    if (inputFrame.empty())
    {
        std::cerr << "Error: Input frame is empty." << std::endl;
        return cv::Mat();
    }

    cv::Mat outputFrame;

    // 2. 定义对比度 (alpha) 乘数
    // 保持 alpha 为 1.0，只调整亮度 (beta)
    double alpha = 1.0;

    // 3. 使用 cv::Mat::convertTo 进行线性变换
    // outputFrame = alpha * inputFrame + beta
    // 这种方法适用于所有通道 (BGR 或 Gray)

    // 注意：
    // - inputFrame.depth() 指定了目标图像的数据类型深度 (例如 CV_8U)。
    // - 我们直接在原图的深度上操作，避免了不必要的类型转换，但需要确保 beta 是整数。
    inputFrame.convertTo(outputFrame, -1, alpha, (double)beta);

    return outputFrame;
}

std::vector<cv::Mat> tools::chopFrame(const std::vector<Armor> &inputArmors, const cv::Mat &inputFrame)
{
    std::vector<cv::Mat> outputFrame;

    return outputFrame;
}

void tools::drawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness)
{
    cv::Point2f vertices[4];
    rotatedRect.points(vertices); // 获取四个顶点
    // 绘制轮廓
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

std::vector<std::vector<cv::Point>> tools::getContours(cv::Mat &inputFrame)
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
    double minAreaThreshold = 200.0; // 设定最小面积阈值（根据你的图像调整）

    for (const auto &contour : contours)
    {
        // 1. 计算轮廓面积
        double area = cv::contourArea(contour);

        // 2. 过滤：如果面积大于阈值，则保留
        if (area > minAreaThreshold)
        {
            filteredContours.push_back(contour);
        }
    }
    // cv::drawContours(inputFrame, filteredContours, -1, cv::Scalar(0, 0, 255), 2);
    return filteredContours;
}

void tools::classifyArmors(int &total, const std::vector<cv::Rect> &boxes, const std::vector<int> &classIds, const std::vector<float> &confidences, std::vector<Robot> &cars, std::vector<Robot> &watchers, std::vector<Armor> &armors)
{
    for (size_t i = 0; i < boxes.size(); ++i)
    {
        if (classIds[i] < 3)
        {
            // 这里先筛选所有装甲
            Armor t;
            t.Box = boxes[i];
            t.confidence = confidences[i];
            t.color = cv::Scalar(maxx(255, classIds[i] * 255), 255 * ((classIds[i]) % 2), minn(255, classIds[i] * 255)); // Actually, this is trickey!
            t.detect_id = total;
            total++;
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

cv::Mat tools::mainFunction(const cv::Mat &inputFrame)
{
    cv::Mat outputframe = inputFrame.clone();
    outputframe = enhanceContrast(inputFrame);
    outputframe = enhanceContrast(outputframe);
    // outputframe = adjustBrightness(inputFrame,10);
    getContours(outputframe);
    return outputframe;
}