#include "../include/pnp_solver.hpp"

void afterDetections::detectNumbers(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){
    //莫亚给出的经验是：tools::enhanceContrast(tools::adjustBrightness(frame_try2,15));
    //拉高Contour的Size 至少300 期望500
    //但是需要编辑frame
    //*iter = tools::enhanceContrast(tools::adjustBrightness(*iter,15));


}

void afterDetections::pnpCalculations(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){

}

void afterDetections::getLightBar(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){


}

void afterDetections::mainCalculations(std::vector<Armor> &inputArmors, cv::Mat &inputFrame){
    if(inputArmors.size()>0){
        
    }

}