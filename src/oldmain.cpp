#include <iostream>
#include "armor.hpp"
#include "structures.hpp"
#include "other_modules.hpp"
#include <opencv2/opencv.hpp>
#include "video_capture.hpp"
//Step1 把assets/下面那个视频拉出来 使用opencv处理成图像然后挂事件
//done ?
//Step2 处理YOLO 
//Step3 OPENCV confirm 装甲
//Step4 预测

int main(int argc, char** argv){
    VideoCaptureWrapper camera_raw("assets/Circular1.avi");
    if(!camera_raw.isOpened()){
        return -1;
    }
    cv::Mat camera_raw_image;
    
    //Step2 Cycle
    while(1){
        //A: Yolo



        //B: OpenCV confirm


        //C: PNP show where?


        //D: Kalman


    }
}