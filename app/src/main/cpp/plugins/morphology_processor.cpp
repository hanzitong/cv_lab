#include "morphology_processor.h"

std::vector<ParamDef> MorphologyProcessor::paramDefs() const {
    return {
        // 0=Erosion 1=Dilation 2=Opening 3=Closing 4=Gradient 5=TopHat 6=BlackHat
        {"operation",   "Operation",    0.f,  6.f,  4.f,  true},
        // 0=RECT 1=CROSS 2=ELLIPSE
        {"kernelShape", "Kernel Shape", 0.f,  2.f,  2.f,  true},
        {"kernelSize",  "Kernel Size",  1.f,  21.f, 5.f,  true},
        {"iterations",  "Iterations",   1.f,  5.f,  1.f,  true},
    };
}

void MorphologyProcessor::setParameter(const std::string& key, float value) {
    if      (key == "operation")   operation_   = static_cast<int>(value);
    else if (key == "kernelShape") kernelShape_ = static_cast<int>(value);
    else if (key == "kernelSize")  kernelSize_  = static_cast<int>(value) | 1;  // 奇数に丸める
    else if (key == "iterations")  iterations_  = static_cast<int>(value);
}

cv::Mat MorphologyProcessor::buildKernel() const {
    int shape;
    switch (kernelShape_) {
        case 1:  shape = cv::MORPH_CROSS;   break;
        case 2:  shape = cv::MORPH_ELLIPSE; break;
        default: shape = cv::MORPH_RECT;    break;
    }
    return cv::getStructuringElement(shape, {kernelSize_, kernelSize_});
}

int MorphologyProcessor::cvMorphOp() const {
    switch (operation_) {
        case 0: return -1;             // Erosion  (erode で処理)
        case 1: return -2;             // Dilation (dilate で処理)
        case 2: return cv::MORPH_OPEN;
        case 3: return cv::MORPH_CLOSE;
        case 4: return cv::MORPH_GRADIENT;
        case 5: return cv::MORPH_TOPHAT;
        case 6: return cv::MORPH_BLACKHAT;
        default: return cv::MORPH_GRADIENT;
    }
}

IProcessor::Output MorphologyProcessor::process(const cv::Mat& gray) {
    Output out;
    out.outputType = OutputType::OVERLAY_IMAGE;

    const auto kernel = buildKernel();
    const int  op     = cvMorphOp();

    if (op == -1) {
        cv::erode(gray, out.overlayImage, kernel, {-1, -1}, iterations_);
    } else if (op == -2) {
        cv::dilate(gray, out.overlayImage, kernel, {-1, -1}, iterations_);
    } else {
        cv::morphologyEx(gray, out.overlayImage, op, kernel,
                         {-1, -1}, iterations_);
    }

    return out;
}
