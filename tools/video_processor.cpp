#include "../include/video_processor.hpp"
#include "../include/armor.hpp"
#include <vector>

void VideoProcessor::DrawRotatedRect(cv::Mat &image, const cv::RotatedRect &rotatedRect, const cv::Scalar &color, int thickness) {
    cv::Point2f vertices[4];
    rotatedRect.points(vertices); // 获取四个顶点
    // 绘制轮廓
    for (int i = 0; i < 4; i++) {
        line(image, vertices[i], vertices[(i + 1) % 4], color, thickness);
    }
}

void VideoProcessor::DrawLightbars(cv::Mat &image, const Lightbar_Pair &inputPairs, const cv::Scalar &color, int thickness){
    DrawRotatedRect(image,inputPairs.left_LightBar,color,thickness);
    DrawRotatedRect(image,inputPairs.right_LightBar,color,thickness);
}
//包装一下

bool VideoProcessor::chk_vaild(cv::RotatedRect &rotrect){
    double ang = rotrect.angle;
    double wid = rotrect.size.width;
    double hei = rotrect.size.height;
    if (hei <= 7 && wid <= 7) return 0; // too small
    if (hei >=100 || wid >= 100) return 0; //too big
    if(ang <= 0) ang = -ang;
    if(ang > 90) ang = 180 - ang;
    if(ang > 0 && ang <= 45) {
        ang = 90 - ang;
        double t = wid;
        wid = hei;
        hei = wid;
    }
    if (hei > 35) return 0;
    double rat = wid/hei;
    if(rat < 3.0 || rat > 15.0) return 0;
    return 1;
}

std::vector<cv::RotatedRect> VideoProcessor::findRect(const cv::Mat& inputFrame){
    cv::Mat gray_img;
    cv::cvtColor(inputFrame,gray_img, cv::COLOR_BGR2GRAY);
    cv::Mat binary_image;
    cv::threshold(gray_img,binary_image,120,255,cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    std::vector<cv::RotatedRect> rotaterects;
    for(const auto & contour : contours){
        auto rotaterect = cv::minAreaRect(contour);
        if(chk_vaild(rotaterect))
            rotaterects.emplace_back(rotaterect);
        
    }  
    return rotaterects;
    //cv::drawContours(processedFrame, contours, -1 ,cv::Scalar(0,0,255),2);
}

std::vector<Lightbar_Pair> VideoProcessor::findPairs(std::vector<cv::RotatedRect> inputRects,const cv::Mat& inputFrame){
    std::vector<cv::RotatedRect> exersiRects;
    for(const auto & rects: inputRects){
        exersiRects.emplace_back(rects);
    }
    std::vector<Lightbar_Pair> out_light;
    auto a = exersiRects.begin();
    auto b = exersiRects.begin();
    
    while(a != exersiRects.end()){
        if(((((*a).center.x)*((*a).center.x))+(((*a).center.y)*((*a).center.y)))>0){
            auto b = exersiRects.begin();
            while(b != exersiRects.end()){
                if(((((*b).center.x)*((*b).center.x))+(((*b).center.y)*((*b).center.y)))>0){  
                cv::Point2f apoint = (*a).center;
                cv::Point2f bpoint = (*b).center;
                double alen = ((*a).size.height>(*a).size.width?(*a).size.height:(*a).size.width);
                double blen = ((*b).size.height>(*b).size.width?(*b).size.height:(*b).size.width);
                double alon = ((*a).size.height<(*a).size.width?(*a).size.height:(*a).size.width);
                double blon = ((*b).size.height<(*b).size.width?(*b).size.height:(*b).size.width);
                double cmpx = (alen + blen)/2.0000000;
                double cmpy = (alon + blon)/2.0000000;
                double disx = apoint.x - bpoint.x;
                double disy = apoint.y - bpoint.y;
                if((disx * disx ) + (disy * disy)>0){
                    /*
                    std::cout<<"Searching for these points"<<std::endl;
                    std::cout<<"A:"<<apoint.x<<" "<<apoint.y<<" "<<alen<<" "<<alon<<std::endl;
                    std::cout<<"B:"<<bpoint.x<<" "<<bpoint.y<<" "<<blen<<" "<<blon<<std::endl;
                    std::cout<<"Expected:"<<cmpx<<" "<<cmpy<<std::endl;
                    std::cout<<"Reality:"<<disx<<" "<<disy<<std::endl;
                    std::cout<<std::endl;
                    */
                    if(((disx < cmpx * 2.500000)&&(disx > cmpx * -2.500000))&&((disy < cmpy * 0.7500)&&(disy > cmpy * -0.7500))){
                        Lightbar_Pair adds;
                        adds.left_LightBar = (*a);
                        adds.right_LightBar = (*b);
                        (*a).center.x = 0;
                        (*b).center.x = 0;
                        (*a).center.y = 0;
                        (*b).center.y = 0;
                        out_light.push_back(adds);
                        //std::cout<<"foundreal"<<std::endl;
                    }
                    }
                }
                b++;
            }
        }
        a++;
    }
    return out_light;
}

cv::Mat VideoProcessor::processFrame(const cv::Mat& inputFrame) {
    cv::Mat processedFrame = inputFrame.clone();
    std::vector<cv::RotatedRect> rotaterects = findRect(inputFrame); 
    //int stk = 0;
    /*
    for(const auto & rectan : rotaterects){
        //std::cout<<rectan.center.x<<" "<<rectan.center.y<<std::endl;
        //DrawRotatedRect(processedFrame,rectan,cv::Scalar(stk*50,255,stk*50),2);
        //stk++;
    }*/
    
    std::vector<Lightbar_Pair> lightbars = findPairs(rotaterects,inputFrame);

    for(const auto & ligpar : lightbars){
        DrawLightbars(processedFrame,ligpar,cv::Scalar(0,255,0),2);
    }
    //std::cout<<std::endl<<lightbars.size()<<std::endl;
    return processedFrame;
}