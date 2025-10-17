#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
//#include "../onnxruntime/include/onnxruntime_cxx_api.h" // 引入 ONNX Runtime
#include "../include/tools.hpp"

using namespace cv;
using namespace std;
const string MODEL_PATH = "models/yolov8n_armor_1280_nms_73.onnx";
//const string VIDEO_PATH = "assets/circular1.avi";
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
            // 注意：使用 unique_ptr 确保 Ort::Session 在析构时被正确释放
            session_ = make_unique<Ort::Session>(env_, model_path.c_str(), session_options_);

            // --- 核心修复：使用 Ort::AllocatedStringPtr 安全获取名称 ---
            // 1. 获取输入名称指针并复制到 std::string
            Ort::AllocatedStringPtr input_name_ptr = session_->GetInputNameAllocated(0, allocator_);
            input_name_str_ = input_name_ptr.get();

            // 2. 获取输出名称指针并复制到 std::string
            Ort::AllocatedStringPtr output_name_ptr = session_->GetOutputNameAllocated(0, allocator_);
            output_name_str_ = output_name_ptr.get();
            // -----------------------------------------------------------------

            cout << "ONNX Runtime model loaded successfully: " << model_path << endl;
            return true;
        }
        catch (const Ort::Exception &e)
        {
            cerr << "ONNX Runtime Error on model loading: " << e.what() << endl;
            return false;
        }
    }

    // 预处理并运行推理
    vector<float> preprocessAndInference(Mat &image)
    {
        // 1. 预处理：调整大小并转换颜色
        Mat resized_image;
        resize(image, resized_image, Size(INPUT_WIDTH, INPUT_HEIGHT));
        // BGR (OpenCV默认) -> RGB (YOLOv8通常需要)
        cvtColor(resized_image, resized_image, COLOR_BGR2RGB);
        resized_image.convertTo(resized_image, CV_32FC3, 1.0f / 255.0f);

        // 2. 转换为 ONNX Runtime 要求的张量格式 (HWC -> CHW)
        vector<float> input_tensor_values(3 * INPUT_WIDTH * INPUT_HEIGHT);
        size_t size_per_channel = INPUT_WIDTH * INPUT_HEIGHT;

        for (int i = 0; i < INPUT_HEIGHT; ++i)
        {
            for (int j = 0; j < INPUT_WIDTH; ++j)
            {
                // HWC -> CHW: R, G, B 分别存储
                input_tensor_values[0 * size_per_channel + i * INPUT_WIDTH + j] = resized_image.at<Vec3f>(i, j)[0];
                input_tensor_values[1 * size_per_channel + i * INPUT_WIDTH + j] = resized_image.at<Vec3f>(i, j)[1];
                input_tensor_values[2 * size_per_channel + i * INPUT_WIDTH + j] = resized_image.at<Vec3f>(i, j)[2];
            }
        }

        // 3. 创建输入张量
        const array<int64_t, 4> input_shape = {1, 3, INPUT_HEIGHT, INPUT_WIDTH};
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            input_tensor_values.data(),
            input_tensor_values.size(),
            input_shape.data(),
            input_shape.size());

        // 4. 推理
        const char *input_name = input_name_str_.c_str();   // 使用 std::string 的 c_str()
        const char *output_name = output_name_str_.c_str(); // 使用 std::string 的 c_str()

        vector<Ort::Value> output_tensors = session_->Run(
            Ort::RunOptions{nullptr},
            &input_name,
            &input_tensor,
            1,
            &output_name,
            1);

        // 5. 返回输出数据 (假设只有一个输出)
        float *raw_output = output_tensors[0].GetTensorMutableData<float>();
        auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
        size_t total_elements = std::accumulate(output_shape.begin(), output_shape.end(), 1, std::multiplies<int64_t>());

        return vector<float>(raw_output, raw_output + total_elements);
    }
    static void post_process_ort(Mat &frame, const vector<float> &output_data,
                                 vector<Rect> &boxes, vector<int> &classIds, vector<float> &confidences);

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    unique_ptr<Ort::Session> session_;
    Ort::AllocatorWithDefaultOptions allocator_;
    std::string input_name_str_;
    std::string output_name_str_;
};