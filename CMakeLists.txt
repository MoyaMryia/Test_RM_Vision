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
project(Test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenCV REQUIRED)
set(ONNXRUNTIME_REL_PATH "onnxruntime") 

# 使用 CMAKE_SOURCE_DIR 构造 ONNX Runtime 库的绝对路径
set(ONNXRUNTIME_ROOT ${CMAKE_SOURCE_DIR}/${ONNXRUNTIME_REL_PATH})

# 检查路径是否存在（可选，但推荐）
if(NOT IS_DIRECTORY ${ONNXRUNTIME_ROOT})
    message(FATAL_ERROR "ONNX Runtime root directory not found at: ${ONNXRUNTIME_ROOT}")
endif()

# 3. 查找共享库文件 (libonnxruntime.so)
find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime 
    HINTS 
        ${ONNXRUNTIME_ROOT}
        ${ONNXRUNTIME_ROOT}/lib
    REQUIRED
)

# 4. 查找头文件 (onnxruntime_cxx_api.h)
find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    HINTS 
        ${ONNXRUNTIME_ROOT}
        ${ONNXRUNTIME_ROOT}/include
    REQUIRED
)

# 检查是否找到
if(NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime library (libonnxruntime.so) not found in ${ONNXRUNTIME_ROOT}.")
endif()

include_directories(
    ${OpenCV_INCLUDE_DIRS} 
    ${ONNXRUNTIME_INCLUDE_DIR}
    include/
)


file(GLOB SOURCES_TOOLS "tools/*.cpp")
file(GLOB SOURCES_SRC "src/*.cpp")
add_executable(Test ${SOURCES_SRC} ${SOURCES_TOOLS})


target_link_libraries(Test 
    PRIVATE 
        ${OpenCV_LIBS} 
        ${ONNXRUNTIME_LIBRARY}
)