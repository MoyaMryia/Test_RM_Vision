#pragma once
#include <../include/armor.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>                            // for std::unique_ptr
#include <opencv2/opencv.hpp>                // 引入 OpenCV 用于图像处理和绘制
#include "../onnxruntime/include/onnxruntime_cxx_api.h" // 引入 ONNX Runtime
#include "../include/yolodetect.hpp"
#include "../include/videoReader.hpp"
#include "../include/failback.hpp"
#include <cmath>
#define pi 3.1415926535897