#include "feature_processor.h"
#include "detectors/sift_detector.h"
#include "detectors/orb_detector.h"
#include "detectors/akaze_detector.h"

#include <opencv2/imgproc.hpp>
#include <chrono>

FeatureProcessor::FeatureProcessor() {
    setAlgorithm(AlgorithmType::ORB);
}

void FeatureProcessor::setAlgorithm(AlgorithmType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentAlgo_ = type;
    switch (type) {
        case AlgorithmType::SIFT:  detector_ = std::make_unique<SIFTDetector>();  break;
        case AlgorithmType::ORB:   detector_ = std::make_unique<ORBDetector>();   break;
        case AlgorithmType::AKAZE: detector_ = std::make_unique<AKAZEDetector>(); break;
    }
}

void FeatureProcessor::setParameter(const std::string& key, float value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (detector_) detector_->setParameter(key, value);
}

ProcessResult FeatureProcessor::processFrame(
        const uint8_t* yuv, int width, int height, int rotationDegrees) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto t0 = std::chrono::steady_clock::now();

    // Y平面をラップ（コピーなし）。回転が必要な場合は別バッファに書き出す
    cv::Mat src(height, width, CV_8UC1, const_cast<uint8_t*>(yuv));
    cv::Mat gray;  // 回転後の画像（回転なしの場合は src を参照）

    switch (rotationDegrees) {
        case 90:  cv::rotate(src, gray, cv::ROTATE_90_CLOCKWISE);        break;
        case 180: cv::rotate(src, gray, cv::ROTATE_180);                  break;
        case 270: cv::rotate(src, gray, cv::ROTATE_90_COUNTERCLOCKWISE);  break;
        default:  gray = src;  break;  // ゼロコピー参照
    }

    ProcessResult result;
    if (detector_) detector_->detect(gray, result.keypoints);

    auto t1 = std::chrono::steady_clock::now();
    result.processingTimeMs =
        std::chrono::duration<float, std::milli>(t1 - t0).count();

    return result;
}
