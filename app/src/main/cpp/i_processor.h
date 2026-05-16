#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

enum class OutputType : int {
    KEYPOINTS     = 0,  // 円形オーバーレイ
    OVERLAY_IMAGE = 1,  // グレースケール画像（Cannyエッジ等）
    POINT_VECTORS = 2,  // 矢印（オプティカルフロー等）
};

struct ParamDef {
    std::string key;
    std::string displayName;
    float       min;
    float       max;
    float       defaultValue;
    bool        isInt;
};

class IProcessor {
public:
    struct Output {
        OutputType                outputType     = OutputType::KEYPOINTS;
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat                   overlayImage;    // CV_8UC1, OVERLAY_IMAGE 用
        std::vector<cv::Point2f>  arrowStarts;     // POINT_VECTORS 用
        std::vector<cv::Point2f>  arrowEnds;
        float                     processingTimeMs = 0.0f;
    };

    virtual ~IProcessor() = default;
    virtual std::string           name()       const = 0;
    virtual OutputType            outputType() const = 0;
    virtual std::vector<ParamDef> paramDefs()  const = 0;
    virtual void                  setParameter(const std::string& key, float value) = 0;
    virtual Output                process(const cv::Mat& gray) = 0;
};
