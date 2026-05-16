#pragma once

#include "i_feature_detector.h"
#include <opencv2/features2d.hpp>

class SIFTDetector : public IFeatureDetector {
public:
    SIFTDetector();
    void detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& keypoints) override;
    void setParameter(const std::string& key, float value) override;

private:
    void rebuild();

    int    nFeatures_      = 500;
    int    nOctaveLayers_  = 3;
    double contrastThresh_ = 0.04;
    double edgeThresh_     = 10.0;
    double sigma_          = 1.6;
    cv::Ptr<cv::SIFT> detector_;
};
