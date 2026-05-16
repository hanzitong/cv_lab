#pragma once

#include "../i_processor.h"
#include <opencv2/features2d.hpp>

class SIFTProcessor : public IProcessor {
public:
    SIFTProcessor();
    std::string           name()       const override { return "SIFT"; }
    OutputType            outputType() const override { return OutputType::KEYPOINTS; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    void rebuild();
    int    nFeatures_      = 500;
    int    nOctaveLayers_  = 3;
    double contrastThresh_ = 0.04;
    double edgeThresh_     = 10.0;
    double sigma_          = 1.6;
    cv::Ptr<cv::SIFT> detector_;
};
