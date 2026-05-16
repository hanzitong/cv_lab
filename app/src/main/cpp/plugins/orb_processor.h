#pragma once

#include "../i_processor.h"
#include <opencv2/features2d.hpp>

class ORBProcessor : public IProcessor {
public:
    ORBProcessor();
    std::string           name()       const override { return "ORB"; }
    OutputType            outputType() const override { return OutputType::KEYPOINTS; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    void rebuild();
    int   nFeatures_     = 500;
    float scaleFactor_   = 1.2f;
    int   nLevels_       = 8;
    int   edgeThresh_    = 31;
    int   fastThreshold_ = 20;
    cv::Ptr<cv::ORB> detector_;
};
