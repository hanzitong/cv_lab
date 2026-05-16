#pragma once

#include "../i_processor.h"

class HoughLinesProcessor : public IProcessor {
public:
    std::string           name()       const override { return "HoughLines"; }
    OutputType            outputType() const override { return OutputType::POINT_VECTORS; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    float rho_           = 1.0f;   // 距離分解能 (px)
    int   threshold_     = 80;     // 直線と判定するための投票数
    int   minLineLength_ = 50;     // 最短ライン長 (px)
    int   maxLineGap_    = 10;     // 連結を許容するギャップ長 (px)
};
