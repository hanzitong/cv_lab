#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "test_helpers.h"
#include "processor_registry.h"

// ─────────────────────────────────────────────────────────────────────────────
// ヘルパー — cv::Mat の生データを渡す
// ─────────────────────────────────────────────────────────────────────────────

static IProcessor::Output runFrame(ProcessorRegistry& reg,
                                    const cv::Mat& gray,
                                    int rotation = 0) {
    return reg.processFrame(gray.data, gray.cols, gray.rows, rotation);
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — プラグイン一覧・選択
// ─────────────────────────────────────────────────────────────────────────────

class ProcessorRegistryTest : public ::testing::Test {
protected:
    ProcessorRegistry reg;
    static constexpr int W = 320, H = 240;
};

TEST_F(ProcessorRegistryTest, ListPlugins_NonEmpty) {
    EXPECT_FALSE(reg.listPlugins().empty());
}

TEST_F(ProcessorRegistryTest, ListPlugins_ContainsORB) {
    auto names = reg.listPlugins();
    EXPECT_NE(std::find(names.begin(), names.end(), "ORB"), names.end());
}

TEST_F(ProcessorRegistryTest, ListPlugins_ContainsSIFT) {
    auto names = reg.listPlugins();
    EXPECT_NE(std::find(names.begin(), names.end(), "SIFT"), names.end());
}

TEST_F(ProcessorRegistryTest, ListPlugins_ContainsAKAZE) {
    auto names = reg.listPlugins();
    EXPECT_NE(std::find(names.begin(), names.end(), "AKAZE"), names.end());
}

TEST_F(ProcessorRegistryTest, ListPlugins_ContainsCanny) {
    auto names = reg.listPlugins();
    EXPECT_NE(std::find(names.begin(), names.end(), "Canny"), names.end());
}

TEST_F(ProcessorRegistryTest, GetParamDefs_ORB_NonEmpty) {
    EXPECT_FALSE(reg.getParamDefs("ORB").empty());
}

TEST_F(ProcessorRegistryTest, GetParamDefs_UnknownPlugin_ReturnsEmpty) {
    EXPECT_TRUE(reg.getParamDefs("NonExistent").empty());
}

TEST_F(ProcessorRegistryTest, GetParamDefs_AllPluginsHaveValidRanges) {
    for (const auto& name : reg.listPlugins()) {
        for (const auto& d : reg.getParamDefs(name)) {
            EXPECT_LT(d.min, d.max)
                << name << "/" << d.key << ": invalid range";
            EXPECT_TRUE(d.defaultValue >= d.min && d.defaultValue <= d.max)
                << name << "/" << d.key << ": default out of range";
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — 基本動作（デフォルト ORB）
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ProcessorRegistryTest, DefaultPlugin_ProcessFrame_DoesNotCrash) {
    EXPECT_NO_THROW(runFrame(reg, TestImage::checkerboard(W, H)));
}

TEST_F(ProcessorRegistryTest, DefaultPlugin_ProcessingTimeIsPositive) {
    auto out = runFrame(reg, TestImage::checkerboard(W, H));
    EXPECT_GT(out.processingTimeMs, 0.f);
}

TEST_F(ProcessorRegistryTest, DefaultPlugin_OutputTypeIsKEYPOINTS) {
    auto out = runFrame(reg, TestImage::checkerboard(W, H));
    EXPECT_EQ(OutputType::KEYPOINTS, out.outputType);
}

TEST_F(ProcessorRegistryTest, DefaultPlugin_DetectsKeypointsOnCheckerboard) {
    auto out = runFrame(reg, TestImage::checkerboard(W, H));
    EXPECT_GT(out.keypoints.size(), 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — selectPlugin + outputType
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ProcessorRegistryTest, SelectPlugin_ORB_DetectsKeypoints) {
    reg.selectPlugin("ORB");
    EXPECT_GT(runFrame(reg, TestImage::checkerboard(W, H)).keypoints.size(), 0u);
}

TEST_F(ProcessorRegistryTest, SelectPlugin_SIFT_DetectsKeypoints) {
    reg.selectPlugin("SIFT");
    EXPECT_GT(runFrame(reg, TestImage::checkerboard(W, H)).keypoints.size(), 0u);
}

TEST_F(ProcessorRegistryTest, SelectPlugin_AKAZE_DetectsKeypoints) {
    reg.selectPlugin("AKAZE");
    EXPECT_GT(runFrame(reg, TestImage::checkerboard(W, H)).keypoints.size(), 0u);
}

TEST_F(ProcessorRegistryTest, SelectPlugin_Canny_ReturnsOVERLAY_IMAGE) {
    reg.selectPlugin("Canny");
    auto out = runFrame(reg, TestImage::checkerboard(W, H));
    EXPECT_EQ(OutputType::OVERLAY_IMAGE, out.outputType);
}

TEST_F(ProcessorRegistryTest, SelectPlugin_Canny_OverlayNotEmpty) {
    reg.selectPlugin("Canny");
    auto out = runFrame(reg, TestImage::checkerboard(W, H));
    EXPECT_FALSE(out.overlayImage.empty());
    EXPECT_EQ(W, out.overlayImage.cols);
    EXPECT_EQ(H, out.overlayImage.rows);
}

TEST_F(ProcessorRegistryTest, SelectPlugin_Unknown_DoesNotCrashOrChange) {
    reg.selectPlugin("ORB");
    auto before = runFrame(reg, TestImage::checkerboard(W, H)).keypoints.size();

    reg.selectPlugin("NonExistent");  // 現在のプラグインは変わらない
    auto after = runFrame(reg, TestImage::checkerboard(W, H)).keypoints.size();

    EXPECT_EQ(before, after);
}

TEST_F(ProcessorRegistryTest, SelectPlugin_MultipleSwitch_DoesNotCrash) {
    auto names = reg.listPlugins();
    for (int i = 0; i < 3; ++i)
        for (const auto& n : names) {
            reg.selectPlugin(n);
            EXPECT_NO_THROW(runFrame(reg, TestImage::checkerboard(W, H)));
        }
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — キーポイント境界検証
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ProcessorRegistryTest, ORB_KeypointsWithinBounds_NoRotation) {
    reg.selectPlugin("ORB");
    auto out = runFrame(reg, TestImage::checkerboard(W, H), 0);
    for (const auto& kp : out.keypoints) {
        EXPECT_GE(kp.pt.x, 0.f); EXPECT_LT(kp.pt.x, W);
        EXPECT_GE(kp.pt.y, 0.f); EXPECT_LT(kp.pt.y, H);
    }
}

TEST_F(ProcessorRegistryTest, Rotation90_KeypointsWithinSwappedBounds) {
    reg.selectPlugin("ORB");
    auto out = runFrame(reg, TestImage::checkerboard(W, H), 90);
    for (const auto& kp : out.keypoints) {
        EXPECT_GE(kp.pt.x, 0.f); EXPECT_LT(kp.pt.x, H);  // 回転後: 幅=H
        EXPECT_GE(kp.pt.y, 0.f); EXPECT_LT(kp.pt.y, W);  // 回転後: 高さ=W
    }
}

TEST_F(ProcessorRegistryTest, Rotation180_KeypointsWithinOriginalBounds) {
    reg.selectPlugin("ORB");
    auto out = runFrame(reg, TestImage::checkerboard(W, H), 180);
    for (const auto& kp : out.keypoints) {
        EXPECT_GE(kp.pt.x, 0.f); EXPECT_LT(kp.pt.x, W);
        EXPECT_GE(kp.pt.y, 0.f); EXPECT_LT(kp.pt.y, H);
    }
}

TEST_F(ProcessorRegistryTest, Rotation270_KeypointsWithinSwappedBounds) {
    reg.selectPlugin("ORB");
    auto out = runFrame(reg, TestImage::checkerboard(W, H), 270);
    for (const auto& kp : out.keypoints) {
        EXPECT_GE(kp.pt.x, 0.f); EXPECT_LT(kp.pt.x, H);
        EXPECT_GE(kp.pt.y, 0.f); EXPECT_LT(kp.pt.y, W);
    }
}

TEST_F(ProcessorRegistryTest, InvalidRotation_DoesNotCrash) {
    EXPECT_NO_THROW(runFrame(reg, TestImage::checkerboard(W, H), 45));
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — パラメーター設定
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ProcessorRegistryTest, SetParameter_nFeatures10_LimitsORBOutput) {
    reg.selectPlugin("ORB");
    reg.setParameter("nFeatures", 10.f);
    EXPECT_LE(runFrame(reg, TestImage::checkerboard(W, H)).keypoints.size(), 10u);
}

TEST_F(ProcessorRegistryTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(reg.setParameter("doesNotExist", 99.f));
    EXPECT_NO_THROW(runFrame(reg, TestImage::checkerboard(W, H)));
}

TEST_F(ProcessorRegistryTest, SetParameter_Canny_HighThreshold_ReducesEdges) {
    reg.selectPlugin("Canny");
    reg.setParameter("threshold2", 10.f);
    auto more = cv::countNonZero(runFrame(reg, TestImage::checkerboard(W, H)).overlayImage);

    reg.setParameter("threshold2", 250.f);
    auto fewer = cv::countNonZero(runFrame(reg, TestImage::checkerboard(W, H)).overlayImage);

    EXPECT_GE(more, fewer);
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — 連続・安定性
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ProcessorRegistryTest, RepeatedCalls_AreConsistent) {
    reg.selectPlugin("ORB");
    const auto img = TestImage::checkerboard(W, H);
    auto count0 = runFrame(reg, img).keypoints.size();
    for (int i = 0; i < 10; ++i)
        EXPECT_EQ(count0, runFrame(reg, img).keypoints.size()) << "inconsistent at " << i;
}

TEST_F(ProcessorRegistryTest, ProcessFrame_100Frames_DoesNotCrash) {
    for (int i = 0; i < 100; ++i)
        EXPECT_NO_THROW(runFrame(reg, TestImage::checkerboard(W, H)));
}

// ─────────────────────────────────────────────────────────────────────────────
// ProcessorRegistryTest — スレッドセーフティ
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ProcessorRegistryTest, ThreadSafety_ConcurrentSetAndProcess) {
    reg.selectPlugin("ORB");
    std::atomic<bool> stop{false};
    std::atomic<int>  crashes{0};
    const auto img = TestImage::checkerboard(W, H);

    std::thread producer([&]() {
        while (!stop.load()) {
            try { runFrame(reg, img); }
            catch (...) { crashes++; }
        }
    });

    for (int i = 0; i < 50; ++i) {
        reg.setParameter("nFeatures", static_cast<float>(100 + i));
        if (i % 10 == 0) {
            auto names = reg.listPlugins();
            reg.selectPlugin(names[i % names.size()]);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    stop = true;
    producer.join();
    EXPECT_EQ(0, crashes.load()) << "Concurrent access caused a crash/exception";
}

TEST_F(ProcessorRegistryTest, ThreadSafety_TwoProcessFrameThreads) {
    reg.selectPlugin("ORB");
    const auto img = TestImage::checkerboard(W, H);
    std::atomic<int> crashes{0};

    auto worker = [&]() {
        for (int i = 0; i < 30; ++i) {
            try { runFrame(reg, img); }
            catch (...) { crashes++; }
        }
    };

    std::thread t1(worker), t2(worker);
    t1.join(); t2.join();
    EXPECT_EQ(0, crashes.load());
}
