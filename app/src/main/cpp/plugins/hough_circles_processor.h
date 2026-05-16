#pragma once

#include "../i_processor.h"

class HoughCirclesProcessor : public IProcessor {
public:
    std::string           name()       const override { return "HoughCircles"; }
    OutputType            outputType() const override { return OutputType::KEYPOINTS; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    float dp_        = 1.5f;  // 解像度逆比（1=フル解像度）
    float minDist_   = 50.f;  // 円の中心間の最小距離 (px)
    float param1_    = 100.f; // Canny の High Threshold
    float param2_    = 30.f;  // 投票閾値
    int   minRadius_ = 10;    // 最小半径 (px)
    int   maxRadius_ = 200;   // 最大半径 (px)
};
