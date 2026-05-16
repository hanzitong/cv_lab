#pragma once

#include "i_feature_detector.h"
#include <opencv2/features2d.hpp>

class ORBDetector : public IFeatureDetector {
public:
    ORBDetector();
    void detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& keypoints) override;
    void setParameter(const std::string& key, float value) override;

private:
    void rebuild();

    int   nFeatures_     = 500;
    float scaleFactor_   = 1.2f;
    int   nLevels_       = 8;
    int   edgeThresh_    = 31;
    int   fastThreshold_ = 20;
    cv::Ptr<cv::ORB> detector_;
};
