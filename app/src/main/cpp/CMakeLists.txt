cmake_minimum_required(VERSION 3.22.1)
project("cv_feature_lab" CXX)

if(NOT DEFINED OPENCV_SDK_PATH OR OPENCV_SDK_PATH STREQUAL "")
    message(FATAL_ERROR
        "OPENCV_SDK_PATH is not set.\n"
        "Add 'opencv.sdk.path=/path/to/OpenCV-android-sdk' to local.properties")
endif()

set(OpenCV_DIR "${OPENCV_SDK_PATH}/sdk/native/jni")
find_package(OpenCV REQUIRED core imgproc features2d)

add_library(
    cv_feature_lab
    SHARED
    jni_bridge.cpp
    processor_registry.cpp
    plugins/sift_processor.cpp
    plugins/orb_processor.cpp
    plugins/akaze_processor.cpp
    plugins/canny_processor.cpp
    plugins/hough_circles_processor.cpp
    plugins/hough_lines_processor.cpp
    plugins/morphology_processor.cpp
)

target_include_directories(cv_feature_lab PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(cv_feature_lab PRIVATE cxx_std_17)

target_compile_options(cv_feature_lab PRIVATE
    -O2
    -Wall
    -Wextra
    -march=armv8-a+simd
)

target_link_libraries(cv_feature_lab
    ${OpenCV_LIBS}
    android
    log
)
