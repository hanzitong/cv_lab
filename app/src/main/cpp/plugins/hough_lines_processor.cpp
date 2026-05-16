#include "hough_lines_processor.h"
#include <opencv2/imgproc.hpp>

std::vector<ParamDef> HoughLinesProcessor::paramDefs() const {
    return {
        {"rho",           "Rho (px)",        1.f,  5.f,   1.f,  false},
        {"threshold",     "Vote Threshold",  10.f, 200.f, 80.f, true},
        {"minLineLength", "Min Length (px)", 10.f, 300.f, 50.f, true},
        {"maxLineGap",    "Max Gap (px)",    0.f,  100.f, 10.f, true},
    };
}

void HoughLinesProcessor::setParameter(const std::string& key, float value) {
    if      (key == "rho")           rho_           = value;
    else if (key == "threshold")     threshold_     = static_cast<int>(value);
    else if (key == "minLineLength") minLineLength_ = static_cast<int>(value);
    else if (key == "maxLineGap")    maxLineGap_    = static_cast<int>(value);
}

IProcessor::Output HoughLinesProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::POINT_VECTORS;

    cv::Mat blurred, edges;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::Canny(blurred, edges, 50.0, 150.0);

    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, rho_, CV_PI / 180.0,
                    threshold_, minLineLength_, maxLineGap_);

    out.arrowStarts.reserve(lines.size());
    out.arrowEnds.reserve(lines.size());
    for (const auto& l : lines) {
        out.arrowStarts.emplace_back(static_cast<float>(l[0]), static_cast<float>(l[1]));
        out.arrowEnds.emplace_back(static_cast<float>(l[2]), static_cast<float>(l[3]));
    }
    return out;
}
