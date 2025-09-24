#include "../include/video_processor.hpp"

void DrawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness) {
    cv::Point2f vertices[4];
    rotatedRect.points(vertices); // 获取四个顶点
    // 绘制轮廓
    for (int i = 0; i < 4; i++) {
        line(image, vertices[i], vertices[(i + 1) % 4], color, thickness);
    }
}

cv::Mat VideoProcessor::processFrame(const cv::Mat& inputFrame) {
    cv::Mat gray_img;
    cv::cvtColor(inputFrame,gray_img, cv::COLOR_BGR2GRAY);
    cv::Mat binary_image;
    cv::threshold(gray_img,binary_image,120,255,cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<cv::RotatedRect> rotaterects;
    for(const auto & contour : contours){
        auto rotaterect = cv::minAreaRect(contour);
        rotaterects.emplace_back(rotaterect);
    }  
    cv::Mat processedFrame=inputFrame.clone();
    //cv::drawContours(processedFrame, contours, -1 ,cv::Scalar(0,0,255),2);
    for(const auto & rectan : rotaterects){
        DrawRotatedRect(processedFrame,rectan,cv::Scalar(0,255,0),2);
    }
    return processedFrame;
}