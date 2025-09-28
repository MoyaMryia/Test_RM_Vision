// Later init OpenCV
// but I don't have any GPUs
// fuck.
#include <opencv2/opencv.hpp>
#ifndef ARMOR_HPP

#define ARMOR_HPP

struct Lightbar_Pair
{
    cv::RotatedRect left_LightBar;  // 左方灯条，类型为LightBar
    cv::RotatedRect right_LightBar; // 右方灯条，类型为LightBar
};

struct Armor
{
    int class_id;
    float confidence;
    cv::Rect box;
};

struct Armor_out
{
    cv::Rect Box;     // 方形位置，使用cv::Rect来表示装甲板的矩形框
    float confidence; // ResNet识别的置信度
    cv::Scalar color; // 装甲板的颜色，使用cv::Scalar表示（BGR格式）
    int detect_id;    // 自动分配的装甲板ID
    int car_num;      // 根据ResNet识别结果得到的装甲板数字
    Lightbar_Pair Lightbar;
    float priority;       // 评分系统给出的打击评分
    float yaw;            // pnp解算出的偏航角
    cv::Point3f p_camera; // pnp解算出的三维位置信息，包含(x, y, z)
};
#endif