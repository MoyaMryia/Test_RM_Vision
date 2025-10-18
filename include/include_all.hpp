#pragma once

#define _ENEMY 2
#define _FRIENDS 2-_ENEMY
//#define USING_YOLO
#define USING_BACKUP
//#define VIDEO

#if !defined(USING_YOLO) && !defined(USING_BACKUP)
    #error "ERROR: At least one of the USING_YOLO and USING_BACKUP should be declaried."
#endif 

#if !defined(_ENEMY) || ((_ENEMY != 2) && (_ENEMY != 0))
    #error "ERROR: _ENEMY not defined as expected."
#endif

#if !defined(_FRIENDS)
    #warning "WARNING: _FRIENDS not defined."
#endif

#include <../include/armor.hpp>
#include <iostream>
#include <algorithm>
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