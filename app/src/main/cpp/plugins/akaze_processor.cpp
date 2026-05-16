#include "akaze_processor.h"

AKAZEProcessor::AKAZEProcessor() { rebuild(); }

void AKAZEProcessor::rebuild() {
    detector_ = cv::AKAZE::create(
        cv::AKAZE::DESCRIPTOR_MLDB, 0, 3,
        threshold_, nOctaves_, nOctaveLayers_);
}

std::vector<ParamDef> AKAZEProcessor::paramDefs() const {
    return {
        {"threshold",     "Threshold",     0.0001f, 0.01f, 0.001f, false},
        {"nOctaves",      "Octaves",       1.f,     8.f,   4.f,    true},
        {"nOctaveLayers", "Octave Layers", 1.f,     8.f,   4.f,    true},
    };
}

void AKAZEProcessor::setParameter(const std::string& key, float value) {
    if      (key == "threshold")     threshold_     = value;
    else if (key == "nOctaves")      nOctaves_      = static_cast<int>(value);
    else if (key == "nOctaveLayers") nOctaveLayers_ = static_cast<int>(value);
    rebuild();
}

IProcessor::Output AKAZEProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::KEYPOINTS;
    detector_->detect(gray, out.keypoints);
    return out;
}
