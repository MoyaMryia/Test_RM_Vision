#include "../include/pnp_solver.hpp"

void afterDetections::detectNumbers(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){
    std::vector<Armor> outputArmors;
    //莫亚给出的经验是：tools::enhanceContrast(tools::adjustBrightness(frame_try2,15));
    //拉高Contour的Size 至少300 期望500
    std::vector<cv::Mat> computeFrames;
    computeFrames = tools::chopFrame(inputArmors,inputFrame);
    //但是需要编辑frame
    //*iter = tools::enhanceContrast(tools::adjustBrightness(*iter,15));


}

void afterDetections::pnpCalculations(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){
    std::vector<Armor> outputArmors;
    std::vector<cv::Mat> computeFrames;
    computeFrames = tools::chopFrame(inputArmors,inputFrame);


}

void afterDetections::getLightBar(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){
    std::vector<Armor> outputArmors;
    std::vector<cv::Mat> computeFrames;
    computeFrames = tools::chopFrame(inputArmors,inputFrame);
    //这里需要拉低亮度对比度 只抓出来灯条 相机需要防止过曝 镜头要干净不然会翻车
    for(size_t i = 0; i < inputArmors.size(); ++i){
        std::vector<cv::RotatedRect> Lightbars;
        
    }

}

std::vector<Armor> afterDetections::mainCalculations(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){
    std::vector<Armor> outputArmors;
    if(inputArmors.size()>0){
        outputArmors = detectNumbers(inputArmors,inputFrame);
        outputArmors = getLightBar(outputArmors,inputFrame);
        outputArmors = pnpCalculations(outputArmors, inputFrame);
    }

}