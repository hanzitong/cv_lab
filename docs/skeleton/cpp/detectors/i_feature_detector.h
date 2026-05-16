#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

class IFeatureDetector {
public:
    virtual ~IFeatureDetector() = default;
    virtual void detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& keypoints) = 0;
    virtual void setParameter(const std::string& key, float value) = 0;
};
