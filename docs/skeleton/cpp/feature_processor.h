#pragma once

#include "detectors/i_feature_detector.h"
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <opencv2/core.hpp>

enum class AlgorithmType : int {
    SIFT  = 0,
    ORB   = 1,
    AKAZE = 2
};

struct ProcessResult {
    std::vector<cv::KeyPoint> keypoints;
    float processingTimeMs = 0.0f;
};

class FeatureProcessor {
public:
    FeatureProcessor();
    void setAlgorithm(AlgorithmType type);
    void setParameter(const std::string& key, float value);
    ProcessResult processFrame(const uint8_t* yuv, int width, int height, int rotationDegrees);

private:
    std::unique_ptr<IFeatureDetector> detector_;
    AlgorithmType currentAlgo_ = AlgorithmType::ORB;
    std::mutex mutex_;
};
