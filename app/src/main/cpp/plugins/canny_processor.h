#pragma once

#include "../i_processor.h"

class CannyProcessor : public IProcessor {
public:
    std::string           name()       const override { return "Canny"; }
    OutputType            outputType() const override { return OutputType::OVERLAY_IMAGE; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    float threshold1_   = 50.0f;
    float threshold2_   = 150.0f;
    int   apertureSize_ = 3;
};
