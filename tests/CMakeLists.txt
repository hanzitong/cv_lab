cmake_minimum_required(VERSION 3.22.1)
project(cv_feature_lab_tests CXX)

# ホストマシン（Linux）上でOpenCVとGTestを使って実行するC++ユニットテスト
# 使い方:
#   mkdir build_tests && cd build_tests
#   cmake ../tests
#   cmake --build .
#   ctest --output-on-failure

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(OpenCV REQUIRED core imgproc features2d)

enable_testing()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../app/src/main/cpp")

add_executable(cv_feature_lab_tests
    cpp/detector_tests.cpp
    cpp/feature_processor_tests.cpp
    ${SRC_DIR}/processor_registry.cpp
    ${SRC_DIR}/plugins/sift_processor.cpp
    ${SRC_DIR}/plugins/orb_processor.cpp
    ${SRC_DIR}/plugins/akaze_processor.cpp
    ${SRC_DIR}/plugins/canny_processor.cpp
    ${SRC_DIR}/plugins/morphology_processor.cpp
)

target_include_directories(cv_feature_lab_tests PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp
)

target_compile_features(cv_feature_lab_tests PRIVATE cxx_std_17)
target_compile_options(cv_feature_lab_tests PRIVATE -Wall -Wextra)

target_link_libraries(cv_feature_lab_tests
    GTest::gtest_main
    ${OpenCV_LIBS}
)

include(GoogleTest)
gtest_discover_tests(cv_feature_lab_tests)
