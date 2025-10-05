#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
#include <onnxruntime/onnxruntime_cxx_api.h> // 引入 ONNX Runtime
#include "../include/yolodetect.hpp"
#include "../include/video_reader.hpp"
#include "../include/frameprocess.hpp"
#include "../include/tools.hpp"

int main(){
    YOLOv8Detector detector;
    if (!detector.loadModel(MODEL_PATH)){return -1;}
    VideoReader reader(VIDEO_PATH);
    if (!reader.isOpened()){return -1;}
    cv::Mat frame;
    while (true){if (!reader.readFrame(frame)){break;}
        //下面的代码会被拆分到yolodetect.hpp中

        //推理
        std::vector<float> output_data = detector.preprocessAndInference(frame);
        // std::cout << "Inference complete. Detected candidate boxes: " << output_data.size() / 6 << std::endl;
        // output_data的格式如下：
        // [x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id],[x1, y1, x2, y2, confidence, class_id] ...
        //只考虑一个输出是足够的
        //不详细折腾onnx之类的了 能用就行
        //主要是你Intel Core Ultra 9 285H 的算力还是太弱了 上OpenVino也无能为力

        //后处理
        std::vector<Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        YOLOv8Detector::post_process_ort(frame, output_data, boxes, classIds, confidences);

        //to here

        tools::drawDetections(frame, boxes, classIds, confidences);
        //std::cout << "Final detected objects after confidence filter: " << boxes.size() << std::endl;
        cv::namedWindow("YOLOv8 Detection (ONNX Runtime)", cv::WINDOW_NORMAL);
        cv::imshow("YOLOv8 Detection (ONNX Runtime)", frame);
        if (cv::waitKey(25) == 'q' || cv::waitKey(25) == 27){break;}
    }
    cv::destroyAllWindows();

    return 0;
}