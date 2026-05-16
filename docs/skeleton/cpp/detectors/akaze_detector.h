#pragma once

#include "i_feature_detector.h"
#include <opencv2/features2d.hpp>

class AKAZEDetector : public IFeatureDetector {
public:
    AKAZEDetector();
    void detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& keypoints) override;
    void setParameter(const std::string& key, float value) override;

private:
    void rebuild();

    float threshold_     = 0.001f;
    int   nOctaves_      = 4;
    int   nOctaveLayers_ = 4;
    cv::Ptr<cv::AKAZE> detector_;
};
