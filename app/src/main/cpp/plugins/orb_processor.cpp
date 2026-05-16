#include "orb_processor.h"

ORBProcessor::ORBProcessor() { rebuild(); }

void ORBProcessor::rebuild() {
    detector_ = cv::ORB::create(
        nFeatures_, scaleFactor_, nLevels_, edgeThresh_,
        0, 2, cv::ORB::HARRIS_SCORE, 31, fastThreshold_);
}

std::vector<ParamDef> ORBProcessor::paramDefs() const {
    return {
        {"nFeatures",     "Max Features",   100.f,  5000.f, 500.f,  true},
        {"scaleFactor",   "Scale Factor",   1.1f,   2.0f,   1.2f,   false},
        {"nLevels",       "Levels",         1.f,    16.f,   8.f,    true},
        {"edgeThreshold", "Edge Threshold", 1.f,    63.f,   31.f,   true},
        {"fastThreshold", "FAST Threshold", 1.f,    40.f,   20.f,   true},
    };
}

void ORBProcessor::setParameter(const std::string& key, float value) {
    if      (key == "nFeatures")     nFeatures_     = static_cast<int>(value);
    else if (key == "scaleFactor")   scaleFactor_   = value;
    else if (key == "nLevels")       nLevels_       = static_cast<int>(value);
    else if (key == "edgeThreshold") edgeThresh_    = static_cast<int>(value);
    else if (key == "fastThreshold") fastThreshold_ = static_cast<int>(value);
    rebuild();
}

IProcessor::Output ORBProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::KEYPOINTS;
    detector_->detect(gray, out.keypoints);
    return out;
}
