#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
#include "../onnxruntime/include/onnxruntime_cxx_api.h" // 引入 ONNX Runtime
#include "../include/armor.hpp"

using namespace cv;
using namespace std;
const string MODEL_PATH = "models/yolov8n_armor_1280_nms_73.onnx";
const int INPUT_WIDTH = 1280;
const int INPUT_HEIGHT = 1280;
const float CONFIDENCE_THRESHOLD = 0.30f;

class YOLOv8Detector
{
public:
    YOLOv8Detector() : env_(ORT_LOGGING_LEVEL_WARNING, "YOLOv8_Detector"),
                       session_options_()
    {
        session_options_.SetGraphOptimizationLevel(ORT_ENABLE_EXTENDED);
    }

    bool loadModel(const string &model_path)
    {
        try
        {
            session_ = make_unique<Ort::Session>(env_, model_path.c_str(), session_options_);
            Ort::AllocatedStringPtr input_name_ptr = session_->GetInputNameAllocated(0, allocator_);
            input_name_str_ = input_name_ptr.get();
            Ort::AllocatedStringPtr output_name_ptr = session_->GetOutputNameAllocated(0, allocator_);
            output_name_str_ = output_name_ptr.get();
            cout << "ONNX Runtime model loaded successfully: " << model_path << endl;
            return true;
        }
        catch (const Ort::Exception &e)
        {
            cerr << "ONNX Runtime Error on model loading: " << e.what() << endl;
            return false;
        }
    }

    vector<float> preprocessAndInference(Mat &image)
    {
        Mat resized_image;
        resize(image, resized_image, Size(INPUT_WIDTH, INPUT_HEIGHT));
        cvtColor(resized_image, resized_image, COLOR_BGR2RGB);
        resized_image.convertTo(resized_image, CV_32FC3, 1.0f / 255.0f);
        vector<float> input_tensor_values(3 * INPUT_WIDTH * INPUT_HEIGHT);
        size_t size_per_channel = INPUT_WIDTH * INPUT_HEIGHT;

        for (int i = 0; i < INPUT_HEIGHT; ++i)
        {
            for (int j = 0; j < INPUT_WIDTH; ++j)
            {
                input_tensor_values[0 * size_per_channel + i * INPUT_WIDTH + j] = resized_image.at<Vec3f>(i, j)[0];
                input_tensor_values[1 * size_per_channel + i * INPUT_WIDTH + j] = resized_image.at<Vec3f>(i, j)[1];
                input_tensor_values[2 * size_per_channel + i * INPUT_WIDTH + j] = resized_image.at<Vec3f>(i, j)[2];
            }
        }

        const array<int64_t, 4> input_shape = {1, 3, INPUT_HEIGHT, INPUT_WIDTH};
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_tensor_values.data(),
            input_tensor_values.size(),
            input_shape.data(),
            input_shape.size());
        const char *input_name = input_name_str_.c_str();   
        const char *output_name = output_name_str_.c_str(); 

        vector<Ort::Value> output_tensors = session_->Run(
            Ort::RunOptions{nullptr},
            &input_name,
            &input_tensor,
            1,
            &output_name,
            1);

        float *raw_output = output_tensors[0].GetTensorMutableData<float>();
        auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
        size_t total_elements = std::accumulate(output_shape.begin(), output_shape.end(), 1, std::multiplies<int64_t>());

        return vector<float>(raw_output, raw_output + total_elements);
    }
    static void post_process_ort(Mat &frame, const vector<float> &output_data,
                                 vector<Rect> &boxes, vector<int> &classIds, vector<float> &confidences);
    static std::vector<Armor> mainFunction(Mat frame, YOLOv8Detector &detector, long long &total);                             

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    unique_ptr<Ort::Session> session_;
    Ort::AllocatorWithDefaultOptions allocator_;
    std::string input_name_str_;
    std::string output_name_str_;
};