#cmake_minimum_required(VERSION 2.8)
#project( Test )
#find_package( OpenCV REQUIRED )
#include_directories( ${OpenCV_INCLUDE_DIRS} include/)
#file(GLOB SOURCES_TOOLS "tools/*.cpp")
#file(GLOB SOURCES_SRC "src/*.cpp")
#add_executable( Test ${SOURCES_SRC} ${SOURCES_TOOLS} )
#target_link_libraries( Test PRIVATE ${OpenCV_LIBS} )

#source venv_yolov8/bin/activate
cmake_minimum_required(VERSION 3.12)
project(Test)

set(CMAKE_CXX_STANDARD 17)

# 查找必要的包
find_package(OpenCV REQUIRED)
find_package(OpenVINO QUIET)

# 设置ONNX Runtime路径（根据您的安装路径修改）
set(ONNXRUNTIME_ROOT "onnxruntime/" CACHE PATH "ONNX Runtime root directory")

# 查找ONNX Runtime
find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATHS ${ONNXRUNTIME_ROOT}/include
    /usr/include/onnxruntime
    /usr/local/include/onnxruntime
)

find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS ${ONNXRUNTIME_ROOT}/lib
    /usr/lib
    /usr/local/lib
)

# 包含目录
include_directories(${ONNXRUNTIME_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} include/)

# 添加可执行文件
file(GLOB SOURCES_TOOLS "tools/*.cpp")
file(GLOB SOURCES_SRC "src/*.cpp")
#add_executable(yolov8_inference main.cpp)
add_executable( Test ${SOURCES_SRC} ${SOURCES_TOOLS} )

# 链接库
target_link_libraries(Test 
    ${OpenCV_LIBS} 
    ${ONNXRUNTIME_LIBRARY}
)

# 设置属性
set_target_properties(Test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)