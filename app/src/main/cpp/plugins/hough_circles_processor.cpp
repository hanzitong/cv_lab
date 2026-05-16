#include "hough_circles_processor.h"
#include <opencv2/imgproc.hpp>

std::vector<ParamDef> HoughCirclesProcessor::paramDefs() const {
    return {
        {"dp",        "DP (inv ratio)", 1.f,  3.f,   1.5f, false},
        {"minDist",   "Min Dist (px)",  10.f, 300.f, 50.f, false},
        {"param1",    "Canny High",     50.f, 300.f, 100.f,false},
        {"param2",    "Vote Threshold", 10.f, 100.f, 30.f, false},
        {"minRadius", "Min Radius (px)",0.f,  200.f, 10.f, true},
        {"maxRadius", "Max Radius (px)",1.f,  500.f, 200.f,true},
    };
}

void HoughCirclesProcessor::setParameter(const std::string& key, float value) {
    if      (key == "dp")        dp_        = value;
    else if (key == "minDist")   minDist_   = value;
    else if (key == "param1")    param1_    = value;
    else if (key == "param2")    param2_    = value;
    else if (key == "minRadius") minRadius_ = static_cast<int>(value);
    else if (key == "maxRadius") maxRadius_ = static_cast<int>(value);
}

IProcessor::Output HoughCirclesProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::KEYPOINTS;

    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(9, 9), 2.0);

    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred, circles, cv::HOUGH_GRADIENT,
                     dp_, minDist_, param1_, param2_, minRadius_, maxRadius_);

    out.keypoints.reserve(circles.size());
    for (const auto& c : circles) {
        out.keypoints.emplace_back(cv::Point2f(c[0], c[1]), c[2] * 2.f);  // size = diameter
    }
    return out;
}
