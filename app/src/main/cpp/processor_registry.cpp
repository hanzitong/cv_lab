#include "processor_registry.h"
#include "plugins/sift_processor.h"
#include "plugins/orb_processor.h"
#include "plugins/akaze_processor.h"
#include "plugins/canny_processor.h"
#include "plugins/morphology_processor.h"

#include <opencv2/imgproc.hpp>
#include <chrono>

ProcessorRegistry::ProcessorRegistry() {
    registerPlugin(std::make_unique<AKAZEProcessor>());  // map はアルファベット順
    registerPlugin(std::make_unique<CannyProcessor>());
    registerPlugin(std::make_unique<MorphologyProcessor>());
    registerPlugin(std::make_unique<ORBProcessor>());
    registerPlugin(std::make_unique<SIFTProcessor>());

    // デフォルトは ORB
    current_ = plugins_["ORB"].get();
}

void ProcessorRegistry::registerPlugin(std::unique_ptr<IProcessor> p) {
    auto name = p->name();
    plugins_[name] = std::move(p);
}

std::vector<std::string> ProcessorRegistry::listPlugins() const {
    std::vector<std::string> names;
    names.reserve(plugins_.size());
    for (const auto& [name, _] : plugins_) names.push_back(name);
    return names;
}

std::vector<ParamDef> ProcessorRegistry::getParamDefs(const std::string& name) const {
    auto it = plugins_.find(name);
    return (it != plugins_.end()) ? it->second->paramDefs() : std::vector<ParamDef>{};
}

void ProcessorRegistry::selectPlugin(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = plugins_.find(name);
    if (it != plugins_.end()) current_ = it->second.get();
}

void ProcessorRegistry::setParameter(const std::string& key, float value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (current_) current_->setParameter(key, value);
}

IProcessor::Output ProcessorRegistry::processFrame(
        const uint8_t* yuv, int width, int height, int rotationDegrees) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto t0 = std::chrono::steady_clock::now();

    // Y平面をラップ（コピーなし）。回転が必要な場合は別バッファに書き出す（DD-14参照）
    cv::Mat src(height, width, CV_8UC1, const_cast<uint8_t*>(yuv));
    cv::Mat gray;

    switch (rotationDegrees) {
        case 90:  cv::rotate(src, gray, cv::ROTATE_90_CLOCKWISE);        break;
        case 180: cv::rotate(src, gray, cv::ROTATE_180);                  break;
        case 270: cv::rotate(src, gray, cv::ROTATE_90_COUNTERCLOCKWISE);  break;
        default:  gray = src; break;
    }

    IProcessor::Output output;
    if (current_) {
        output = current_->process(gray);
        output.outputType = current_->outputType();
    }

    auto t1 = std::chrono::steady_clock::now();
    output.processingTimeMs = std::chrono::duration<float, std::milli>(t1 - t0).count();

    return output;
}
