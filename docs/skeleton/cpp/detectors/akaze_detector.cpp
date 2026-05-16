#include "akaze_detector.h"

AKAZEDetector::AKAZEDetector() { rebuild(); }

void AKAZEDetector::rebuild() {
    detector_ = cv::AKAZE::create(
        cv::AKAZE::DESCRIPTOR_MLDB, 0, 3,
        threshold_, nOctaves_, nOctaveLayers_);
}

void AKAZEDetector::detect(const cv::Mat& gray, std::vector<cv::KeyPoint>& kps) {
    detector_->detect(gray, kps);
}

void AKAZEDetector::setParameter(const std::string& key, float value) {
    if      (key == "threshold")     threshold_    = value;
    else if (key == "nOctaves")      nOctaves_     = static_cast<int>(value);
    else if (key == "nOctaveLayers") nOctaveLayers_ = static_cast<int>(value);
    rebuild();
}
