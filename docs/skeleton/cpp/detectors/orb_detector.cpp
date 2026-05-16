#include "orb_detector.h"

ORBDetector::ORBDetector() { rebuild(); }

void ORBDetector::rebuild() {
    // scoreType=HARRIS_SCORE(0), patchSize=31 are fixed defaults
    detector_ = cv::ORB::create(
        nFeatures_, scaleFactor_, nLevels_, edgeThresh_,
        0, 2, cv::ORB::HARRIS_SCORE, 31, fastThreshold_);
}

void ORBDetector::detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& kps) {
    detector_->detect(gray, kps);
}

void ORBDetector::setParameter(const std::string& key, float value) {
    if      (key == "nFeatures")     nFeatures_     = static_cast<int>(value);
    else if (key == "scaleFactor")   scaleFactor_   = value;
    else if (key == "nLevels")       nLevels_       = static_cast<int>(value);
    else if (key == "edgeThreshold") edgeThresh_    = static_cast<int>(value);
    else if (key == "fastThreshold") fastThreshold_ = static_cast<int>(value);
    rebuild();
}
