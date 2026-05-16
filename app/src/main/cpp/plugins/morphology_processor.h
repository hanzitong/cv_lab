#pragma once

#include "../i_processor.h"
#include <opencv2/imgproc.hpp>

class MorphologyProcessor : public IProcessor {
public:
    std::string           name()       const override { return "Morphology"; }
    OutputType            outputType() const override { return OutputType::OVERLAY_IMAGE; }
    std::vector<ParamDef> paramDefs()  const override;
    void                  setParameter(const std::string& key, float value) override;
    Output                process(const cv::Mat& gray) override;

private:
    // 0=Erosion 1=Dilation 2=Opening 3=Closing 4=Gradient 5=TopHat 6=BlackHat
    int operation_   = 4;  // デフォルト: Gradient（変化が視覚的にわかりやすい）
    int kernelSize_  = 5;  // 構造要素サイズ（奇数に丸める）
    int iterations_  = 1;
    // 0=RECT 1=CROSS 2=ELLIPSE
    int kernelShape_ = 2;

    cv::Mat buildKernel() const;
    int     cvMorphOp()   const;
};
