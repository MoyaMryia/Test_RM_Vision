#include "../include/tools.hpp"

float maxx(float a, float b){
    if (a > b) return a;
    else return b;
}
float minn(float a, float b){
    if (a < b) return a;
    else return b;
}

int main(){
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH)){return -1;}
    VideoReader reader(VIDEO_PATH);
    if (!reader.isOpened()){return -1;}
    cv::Mat frame;
    long long total = 1;
    while (true){if (!reader.readFrame(frame)){break;}
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...

        //后处理
        std::vector<Rect> boxes; std::vector<int> classIds; std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);
        std::vector<Armor> armors;
        for(size_t i = 0;i<boxes.size();++i){
            if(classIds[i] < 3){
                //这里先筛选所有装甲
                Armor t;
                t.Box = boxes[i];
                t.confidence = confidences[i];
                t.color = cv::Scalar(maxx(255,classIds[i]*255),255*((classIds[i])%2),minn(255,classIds[i]*255));//Actually, this is trickey!
                t.detect_id = total;
                total++;
                armors.push_back(t);
            }
        }
        std::vector<cv::Mat> outputFrame;
        outputFrame = tools::chopFrame(armors, frame);
        //There should be something that can be treated;
        //A. afterDetections::mainCalculations
        //B. create a std::vector<Robot> and take the armors into it;
        //C. Kalman Calculations
        //D. And finally, get a mark, done.
        //

        //Treated Done.
        tools::drawDetections(frame, boxes, classIds, confidences);
        cv::namedWindow("YOLOv8 Detection (ONNX Runtime)", cv::WINDOW_NORMAL);
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27){break;}
    }
    cv::destroyAllWindows();
    return 0;
}