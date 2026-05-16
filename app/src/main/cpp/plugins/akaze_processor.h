#pragma once

#include "../i_processor.h"
#include <opencv2/features2d.hpp>

class AKAZEProcessor : public IProcessor {
public:
    AKAZEProcessor();
    std::string           name()       const override { return "AKAZE"; }
    OutputType            outputType() const override { return OutputType::KEYPOINTS; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    void rebuild();
    float threshold_     = 0.001f;
    int   nOctaves_      = 4;
    int   nOctaveLayers_ = 4;
    cv::Ptr<cv::AKAZE> detector_;
};
