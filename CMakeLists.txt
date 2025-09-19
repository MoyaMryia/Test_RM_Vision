cmake_minimum_required(VERSION 3.11)

project(NJU_RM)
include_directories( ${OpenCV_INCLUDE_DIRS} /include ./tasks ./tools)
find_package( OpenCV REQUIRED )
target_link_libraries( NJU_RM ${OpenCV_LIBS} )

#add_executable(Output src/decision_maker.cpp detector.cpp pnp_solver.cpp ros2_manager.cpp tracker.cpp)
