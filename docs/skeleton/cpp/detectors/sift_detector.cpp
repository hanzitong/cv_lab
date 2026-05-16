#include "sift_detector.h"

SIFTDetector::SIFTDetector() { rebuild(); }

void SIFTDetector::rebuild() {
    detector_ = cv::SIFT::create(
        nFeatures_, nOctaveLayers_, contrastThresh_, edgeThresh_, sigma_);
}

void SIFTDetector::detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& kps) {
    detector_->detect(gray, kps);
}

void SIFTDetector::setParameter(const std::string& key, float value) {
    if      (key == "nFeatures")         nFeatures_      = static_cast<int>(value);
    else if (key == "nOctaveLayers")     nOctaveLayers_  = static_cast<int>(value);
    else if (key == "contrastThreshold") contrastThresh_ = static_cast<double>(value);
    else if (key == "edgeThreshold")     edgeThresh_     = static_cast<double>(value);
    else if (key == "sigma")             sigma_          = static_cast<double>(value);
    rebuild();
}
