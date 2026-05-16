#include "canny_processor.h"
#include <opencv2/imgproc.hpp>

std::vector<ParamDef> CannyProcessor::paramDefs() const {
    return {
        {"threshold1",   "Low Threshold",  0.f,  300.f, 50.f,  false},
        {"threshold2",   "High Threshold", 0.f,  300.f, 150.f, false},
        {"apertureSize", "Aperture Size",  3.f,  7.f,   3.f,   true},
    };
}

void CannyProcessor::setParameter(const std::string& key, float value) {
    if      (key == "threshold1")   threshold1_   = value;
    else if (key == "threshold2")   threshold2_   = value;
    else if (key == "apertureSize") apertureSize_ = static_cast<int>(value) | 1;  // 奇数に丸める
}

IProcessor::Output CannyProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::OVERLAY_IMAGE;
    cv::Canny(gray, out.overlayImage, threshold1_, threshold2_, apertureSize_);
    return out;
}
