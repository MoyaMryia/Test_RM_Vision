#include "../include/VideoProcessor.hpp"

cv::Mat VideoProcessor::processFrame(const cv::Mat& inputFrame) {
/*
    cv::Mat grayFrame;
    cv::cvtColor(inputFrame, grayFrame, cv::COLOR_BGR2GRAY);
    */
    cv::Mat gray_img;
    cv::cvtColor(inputFrame,gray_img, cv::COLOR_BGR2GRAY);
    //cv::GaussianBlur(grayFrame, processedFrame, cv::Size(5, 5), 0);
    cv::Mat binary_image;
    cv::threshold(gray_img,binary_image,120,255,cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<cv::RotatedRect> rotaterects;
    for(const auto & contour : contours){
        auto rotaterect = cv::minAreaRect(contour);
        rotaterects.emplace_back(rotaterect);
    }  
    cv::Mat processedFrame=inputFrame;
    cv::drawContours(processedFrame, contours, -1 ,cv::Scalar(0,0,255),2);
    return processedFrame;
}