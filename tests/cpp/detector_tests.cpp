#include <gtest/gtest.h>
#include "test_helpers.h"
#include "plugins/sift_processor.h"
#include "plugins/orb_processor.h"
#include "plugins/akaze_processor.h"
#include "plugins/canny_processor.h"

// ─────────────────────────────────────────────────────────────────────────────
// 共通ヘルパー
// ─────────────────────────────────────────────────────────────────────────────

static bool allWithinBounds(const std::vector<cv::KeyPoint>& kps, int w, int h) {
    for (const auto& kp : kps)
        if (kp.pt.x < 0 || kp.pt.x >= w || kp.pt.y < 0 || kp.pt.y >= h)
            return false;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// ORBProcessor
// ─────────────────────────────────────────────────────────────────────────────

class ORBProcessorTest : public ::testing::Test {
protected:
    ORBProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(ORBProcessorTest, Name_IsORB) {
    EXPECT_EQ("ORB", proc.name());
}

TEST_F(ORBProcessorTest, OutputType_IsKEYPOINTS) {
    EXPECT_EQ(OutputType::KEYPOINTS, proc.outputType());
}

TEST_F(ORBProcessorTest, ParamDefs_NonEmpty) {
    EXPECT_FALSE(proc.paramDefs().empty());
}

TEST_F(ORBProcessorTest, ParamDefs_AllRangesValid) {
    for (const auto& d : proc.paramDefs())
        EXPECT_LT(d.min, d.max) << "param " << d.key << " has invalid range";
}

TEST_F(ORBProcessorTest, ParamDefs_DefaultsInRange) {
    for (const auto& d : proc.paramDefs())
        EXPECT_TRUE(d.defaultValue >= d.min && d.defaultValue <= d.max)
            << "param " << d.key << " default out of range";
}

TEST_F(ORBProcessorTest, DetectsKeypointsOnCheckerboard) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_GT(out.keypoints.size(), 0u);
}

TEST_F(ORBProcessorTest, UniformImage_ReturnsZeroOrFewKeypoints) {
    auto out = proc.process(TestImage::uniform(W, H));
    EXPECT_LT(out.keypoints.size(), 10u);
}

TEST_F(ORBProcessorTest, KeypointsAreWithinFrameBounds) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_TRUE(allWithinBounds(out.keypoints, W, H));
}

TEST_F(ORBProcessorTest, SetParameter_nFeatures_LimitsOutput) {
    proc.setParameter("nFeatures", 10.f);
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_LE(out.keypoints.size(), 10u);
}

TEST_F(ORBProcessorTest, SetParameter_nFeatures_LargerAllowsMore) {
    proc.setParameter("nFeatures", 2000.f);
    auto large = proc.process(TestImage::checkerboard(W, H)).keypoints.size();

    proc.setParameter("nFeatures", 10.f);
    auto small = proc.process(TestImage::checkerboard(W, H)).keypoints.size();

    EXPECT_GE(large, small);
}

TEST_F(ORBProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("nonExistentKey", 42.f));
    EXPECT_NO_THROW(proc.process(TestImage::checkerboard(W, H)));
}

TEST_F(ORBProcessorTest, MultipleProcessCalls_AreConsistent) {
    const auto img = TestImage::checkerboard(W, H);
    auto size1 = proc.process(img).keypoints.size();
    auto size2 = proc.process(img).keypoints.size();
    EXPECT_EQ(size1, size2);
}

// ─────────────────────────────────────────────────────────────────────────────
// SIFTProcessor
// ─────────────────────────────────────────────────────────────────────────────

class SIFTProcessorTest : public ::testing::Test {
protected:
    SIFTProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(SIFTProcessorTest, Name_IsSIFT) {
    EXPECT_EQ("SIFT", proc.name());
}

TEST_F(SIFTProcessorTest, OutputType_IsKEYPOINTS) {
    EXPECT_EQ(OutputType::KEYPOINTS, proc.outputType());
}

TEST_F(SIFTProcessorTest, ParamDefs_HasNFeatures) {
    auto defs = proc.paramDefs();
    auto it = std::find_if(defs.begin(), defs.end(),
        [](const ParamDef& d){ return d.key == "nFeatures"; });
    EXPECT_NE(it, defs.end());
}

TEST_F(SIFTProcessorTest, DetectsKeypointsOnCheckerboard) {
    EXPECT_GT(proc.process(TestImage::checkerboard(W, H)).keypoints.size(), 0u);
}

TEST_F(SIFTProcessorTest, UniformImage_ReturnsZeroOrFewKeypoints) {
    EXPECT_LT(proc.process(TestImage::uniform(W, H)).keypoints.size(), 10u);
}

TEST_F(SIFTProcessorTest, KeypointsAreWithinFrameBounds) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_TRUE(allWithinBounds(out.keypoints, W, H));
}

TEST_F(SIFTProcessorTest, SetParameter_LowContrastThreshold_IncreasesCount) {
    proc.setParameter("contrastThreshold", 0.01f);
    auto low = proc.process(TestImage::checkerboard(W, H)).keypoints.size();

    proc.setParameter("contrastThreshold", 0.09f);
    auto high = proc.process(TestImage::checkerboard(W, H)).keypoints.size();

    EXPECT_GE(low, high);
}

TEST_F(SIFTProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("unknown", 0.f));
}

// ─────────────────────────────────────────────────────────────────────────────
// AKAZEProcessor
// ─────────────────────────────────────────────────────────────────────────────

class AKAZEProcessorTest : public ::testing::Test {
protected:
    AKAZEProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(AKAZEProcessorTest, Name_IsAKAZE) {
    EXPECT_EQ("AKAZE", proc.name());
}

TEST_F(AKAZEProcessorTest, OutputType_IsKEYPOINTS) {
    EXPECT_EQ(OutputType::KEYPOINTS, proc.outputType());
}

TEST_F(AKAZEProcessorTest, DetectsKeypointsOnCheckerboard) {
    EXPECT_GT(proc.process(TestImage::checkerboard(W, H)).keypoints.size(), 0u);
}

TEST_F(AKAZEProcessorTest, UniformImage_ReturnsZeroOrFewKeypoints) {
    EXPECT_LT(proc.process(TestImage::uniform(W, H)).keypoints.size(), 10u);
}

TEST_F(AKAZEProcessorTest, SetParameter_LowThreshold_IncreasesCount) {
    proc.setParameter("threshold", 0.0001f);
    auto low = proc.process(TestImage::checkerboard(W, H)).keypoints.size();

    proc.setParameter("threshold", 0.009f);
    auto high = proc.process(TestImage::checkerboard(W, H)).keypoints.size();

    EXPECT_GE(low, high);
}

TEST_F(AKAZEProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("invalid", 1.f));
}

// ─────────────────────────────────────────────────────────────────────────────
// CannyProcessor
// ─────────────────────────────────────────────────────────────────────────────

class CannyProcessorTest : public ::testing::Test {
protected:
    CannyProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(CannyProcessorTest, Name_IsCanny) {
    EXPECT_EQ("Canny", proc.name());
}

TEST_F(CannyProcessorTest, OutputType_IsOVERLAY_IMAGE) {
    EXPECT_EQ(OutputType::OVERLAY_IMAGE, proc.outputType());
}

TEST_F(CannyProcessorTest, ParamDefs_HasThreshold1AndThreshold2) {
    auto defs = proc.paramDefs();
    auto it1 = std::find_if(defs.begin(), defs.end(),
        [](const ParamDef& d){ return d.key == "threshold1"; });
    auto it2 = std::find_if(defs.begin(), defs.end(),
        [](const ParamDef& d){ return d.key == "threshold2"; });
    EXPECT_NE(it1, defs.end());
    EXPECT_NE(it2, defs.end());
}

TEST_F(CannyProcessorTest, Process_CheckerboardProducesEdges) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_FALSE(out.overlayImage.empty());
    // エッジ画素（255）が存在すること
    double maxVal;
    cv::minMaxLoc(out.overlayImage, nullptr, &maxVal);
    EXPECT_GT(maxVal, 0.0);
}

TEST_F(CannyProcessorTest, Process_UniformImageProducesNoEdges) {
    auto out = proc.process(TestImage::uniform(W, H));
    EXPECT_FALSE(out.overlayImage.empty());
    double maxVal;
    cv::minMaxLoc(out.overlayImage, nullptr, &maxVal);
    EXPECT_EQ(0.0, maxVal);
}

TEST_F(CannyProcessorTest, Process_OverlayImageDimensionsMatchInput) {
    auto img = TestImage::checkerboard(W, H);
    auto out = proc.process(img);
    EXPECT_EQ(W, out.overlayImage.cols);
    EXPECT_EQ(H, out.overlayImage.rows);
}

TEST_F(CannyProcessorTest, Process_OverlayImageIsGrayscale) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_EQ(1, out.overlayImage.channels());
    EXPECT_EQ(CV_8UC1, out.overlayImage.type());
}

TEST_F(CannyProcessorTest, Process_KeypointsAreEmpty) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_TRUE(out.keypoints.empty());
}

TEST_F(CannyProcessorTest, SetParameter_HighThreshold2_ReducesEdges) {
    proc.setParameter("threshold2", 10.f);
    auto moreEdges = cv::countNonZero(
        proc.process(TestImage::checkerboard(W, H)).overlayImage);

    proc.setParameter("threshold2", 250.f);
    auto fewerEdges = cv::countNonZero(
        proc.process(TestImage::checkerboard(W, H)).overlayImage);

    EXPECT_GE(moreEdges, fewerEdges);
}

TEST_F(CannyProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("unknown", 1.f));
}
