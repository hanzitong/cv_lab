#pragma once

#include "i_processor.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>

class ProcessorRegistry {
public:
    ProcessorRegistry();

    std::vector<std::string> listPlugins() const;
    std::vector<ParamDef>    getParamDefs(const std::string& name) const;

    void selectPlugin(const std::string& name);
    void setParameter(const std::string& key, float value);

    // CameraX 分析スレッドから呼ばれる
    IProcessor::Output processFrame(
        const uint8_t* yuv, int width, int height, int rotationDegrees);

private:
    // map はアルファベット順で安定した一覧順序を保証する
    std::map<std::string, std::unique_ptr<IProcessor>> plugins_;
    IProcessor* current_ = nullptr;
    mutable std::mutex mutex_;

    void registerPlugin(std::unique_ptr<IProcessor> p);
};
