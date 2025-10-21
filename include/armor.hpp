#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#ifndef ARMOR_HPP

#define ARMOR_HPP

struct Lightbar_Pair{
  cv::RotatedRect left_LightBar;
  cv::RotatedRect right_LightBar;
};

struct Armor
{
  cv::Rect Box;     // 方形位置，使用cv::Rect来表示装甲板的矩形框
  float confidence; // YOLO识别的置信度
  cv::Scalar color; // 装甲板的颜色，使用cv::Scalar表示（BGR格式）
  int detect_id;    // 自动分配的装甲板ID
  int car_num;      // 根据ResNet识别结果得到的装甲板数字
  Lightbar_Pair Lightbars;
  std::vector<cv::Point2f> position;
  float priority;       // 评分系统给出的打击评分
  float yaw;            // pnp解算出的偏航角
  cv::Point3f p_camera; // pnp解算出的三维位置信息，包含(x, y, z)
  int classId;
};
struct Robot
{
  std::vector<Armor> armor; // 机器人所包含的Armor类
  cv::Rect robotrect;
  float omega;      // 机器人旋转的角速度
  float vx;         // 机器人在水平方向的线速度
  float vz;         // 机器人在垂直方向的线速度
  int carorwatcher; // 是机器人还是哨兵
  cv::Scalar color;
  float confidence;
};
#endif