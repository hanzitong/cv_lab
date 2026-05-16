#include "sift_processor.h"

SIFTProcessor::SIFTProcessor() { rebuild(); }

void SIFTProcessor::rebuild() {
    detector_ = cv::SIFT::create(
        nFeatures_, nOctaveLayers_, contrastThresh_, edgeThresh_, sigma_);
}

std::vector<ParamDef> SIFTProcessor::paramDefs() const {
    return {
        {"nFeatures",         "Max Features",       0.f,    2000.f,  500.f,  true},
        {"nOctaveLayers",     "Octave Layers",      1.f,    8.f,     3.f,    true},
        {"contrastThreshold", "Contrast Threshold", 0.01f,  0.10f,   0.04f,  false},
        {"edgeThreshold",     "Edge Threshold",     1.f,    30.f,    10.f,   true},
        {"sigma",             "Sigma",              0.5f,   3.0f,    1.6f,   false},
    };
}

void SIFTProcessor::setParameter(const std::string& key, float value) {
    if      (key == "nFeatures")         nFeatures_      = static_cast<int>(value);
    else if (key == "nOctaveLayers")     nOctaveLayers_  = static_cast<int>(value);
    else if (key == "contrastThreshold") contrastThresh_ = static_cast<double>(value);
    else if (key == "edgeThreshold")     edgeThresh_     = static_cast<double>(value);
    else if (key == "sigma")             sigma_          = static_cast<double>(value);
    rebuild();
}

IProcessor::Output SIFTProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::KEYPOINTS;
    detector_->detect(gray, out.keypoints);
    return out;
}
