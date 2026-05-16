#include <gtest/gtest.h>
#include "test_helpers.h"
#include "plugins/sift_processor.h"
#include "plugins/orb_processor.h"
#include "plugins/akaze_processor.h"
#include "plugins/canny_processor.h"
#include "plugins/morphology_processor.h"
#include "plugins/hough_lines_processor.h"
#include "plugins/hough_circles_processor.h"

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

// ─────────────────────────────────────────────────────────────────────────────
// MorphologyProcessor
// ─────────────────────────────────────────────────────────────────────────────

class MorphologyProcessorTest : public ::testing::Test {
protected:
    MorphologyProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(MorphologyProcessorTest, Name_IsMorphology) {
    EXPECT_EQ("Morphology", proc.name());
}

TEST_F(MorphologyProcessorTest, OutputType_IsOVERLAY_IMAGE) {
    EXPECT_EQ(OutputType::OVERLAY_IMAGE, proc.outputType());
}

TEST_F(MorphologyProcessorTest, ParamDefs_HasRequiredKeys) {
    auto defs = proc.paramDefs();
    auto has = [&](const std::string& k) {
        return std::any_of(defs.begin(), defs.end(),
            [&](const ParamDef& d){ return d.key == k; });
    };
    EXPECT_TRUE(has("operation"));
    EXPECT_TRUE(has("kernelSize"));
    EXPECT_TRUE(has("iterations"));
    EXPECT_TRUE(has("kernelShape"));
}

TEST_F(MorphologyProcessorTest, ParamDefs_AllRangesValid) {
    for (const auto& d : proc.paramDefs())
        EXPECT_LT(d.min, d.max) << "param " << d.key << " has invalid range";
}

TEST_F(MorphologyProcessorTest, Process_ReturnsNonEmptyOverlay) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_FALSE(out.overlayImage.empty());
}

TEST_F(MorphologyProcessorTest, Process_OverlayDimensionsMatchInput) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_EQ(W, out.overlayImage.cols);
    EXPECT_EQ(H, out.overlayImage.rows);
}

TEST_F(MorphologyProcessorTest, Process_OverlayIsGrayscale) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_EQ(1, out.overlayImage.channels());
    EXPECT_EQ(CV_8UC1, out.overlayImage.type());
}

TEST_F(MorphologyProcessorTest, Process_KeypointsAreEmpty) {
    EXPECT_TRUE(proc.process(TestImage::checkerboard(W, H)).keypoints.empty());
}

TEST_F(MorphologyProcessorTest, Erosion_ShrinksBrightRegions) {
    proc.setParameter("operation", 0.f);   // Erosion
    proc.setParameter("kernelSize", 5.f);
    auto out = proc.process(TestImage::checkerboard(W, H));
    // 収縮後は明部ピクセルが減る → 平均輝度が元より低い or 等しい
    double meanBefore = cv::mean(TestImage::checkerboard(W, H))[0];
    double meanAfter  = cv::mean(out.overlayImage)[0];
    EXPECT_LE(meanAfter, meanBefore + 1.0);  // 浮動小数点誤差を許容
}

TEST_F(MorphologyProcessorTest, Dilation_ExpandsBrightRegions) {
    proc.setParameter("operation", 1.f);   // Dilation
    proc.setParameter("kernelSize", 5.f);
    auto out = proc.process(TestImage::checkerboard(W, H));
    double meanBefore = cv::mean(TestImage::checkerboard(W, H))[0];
    double meanAfter  = cv::mean(out.overlayImage)[0];
    EXPECT_GE(meanAfter, meanBefore - 1.0);
}

TEST_F(MorphologyProcessorTest, Gradient_CheckerboardHasEdges) {
    proc.setParameter("operation", 4.f);   // Gradient = Dilation - Erosion
    auto out = proc.process(TestImage::checkerboard(W, H));
    double maxVal;
    cv::minMaxLoc(out.overlayImage, nullptr, &maxVal);
    EXPECT_GT(maxVal, 0.0);
}

TEST_F(MorphologyProcessorTest, Gradient_UniformImageIsAllZero) {
    proc.setParameter("operation", 4.f);
    auto out = proc.process(TestImage::uniform(W, H));
    double maxVal;
    cv::minMaxLoc(out.overlayImage, nullptr, &maxVal);
    EXPECT_EQ(0.0, maxVal);
}

TEST_F(MorphologyProcessorTest, AllOperations_DoNotCrash) {
    const auto img = TestImage::checkerboard(W, H);
    for (int op = 0; op <= 6; ++op) {
        proc.setParameter("operation", static_cast<float>(op));
        EXPECT_NO_THROW(proc.process(img)) << "operation=" << op;
    }
}

TEST_F(MorphologyProcessorTest, AllKernelShapes_DoNotCrash) {
    const auto img = TestImage::checkerboard(W, H);
    for (int shape = 0; shape <= 2; ++shape) {
        proc.setParameter("kernelShape", static_cast<float>(shape));
        EXPECT_NO_THROW(proc.process(img)) << "kernelShape=" << shape;
    }
}

TEST_F(MorphologyProcessorTest, LargerKernelSize_DoesNotCrash) {
    proc.setParameter("kernelSize", 21.f);
    EXPECT_NO_THROW(proc.process(TestImage::checkerboard(W, H)));
}

TEST_F(MorphologyProcessorTest, MultipleIterations_DoNotCrash) {
    proc.setParameter("iterations", 5.f);
    EXPECT_NO_THROW(proc.process(TestImage::checkerboard(W, H)));
}

TEST_F(MorphologyProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("unknown", 1.f));
}

// ─────────────────────────────────────────────────────────────────────────────
// HoughLinesProcessor
// ─────────────────────────────────────────────────────────────────────────────

class HoughLinesProcessorTest : public ::testing::Test {
protected:
    HoughLinesProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(HoughLinesProcessorTest, Name_IsHoughLines) {
    EXPECT_EQ("HoughLines", proc.name());
}

TEST_F(HoughLinesProcessorTest, OutputType_IsPOINT_VECTORS) {
    EXPECT_EQ(OutputType::POINT_VECTORS, proc.outputType());
}

TEST_F(HoughLinesProcessorTest, ParamDefs_HasRequiredKeys) {
    auto defs = proc.paramDefs();
    auto has = [&](const std::string& k) {
        return std::any_of(defs.begin(), defs.end(),
            [&](const ParamDef& d){ return d.key == k; });
    };
    EXPECT_TRUE(has("rho"));
    EXPECT_TRUE(has("threshold"));
    EXPECT_TRUE(has("minLineLength"));
    EXPECT_TRUE(has("maxLineGap"));
}

TEST_F(HoughLinesProcessorTest, ParamDefs_AllRangesValid) {
    for (const auto& d : proc.paramDefs())
        EXPECT_LT(d.min, d.max) << "param " << d.key << " has invalid range";
}

TEST_F(HoughLinesProcessorTest, ParamDefs_DefaultsInRange) {
    for (const auto& d : proc.paramDefs())
        EXPECT_TRUE(d.defaultValue >= d.min && d.defaultValue <= d.max)
            << "param " << d.key << " default out of range";
}

TEST_F(HoughLinesProcessorTest, Process_StripedImage_DetectsLines) {
    proc.setParameter("threshold", 30.f);
    proc.setParameter("minLineLength", 20.f);
    auto out = proc.process(TestImage::striped(W, H));
    EXPECT_GT(out.arrowStarts.size(), 0u);
}

TEST_F(HoughLinesProcessorTest, Process_UniformImage_NoLines) {
    auto out = proc.process(TestImage::uniform(W, H));
    EXPECT_EQ(out.arrowStarts.size(), 0u);
}

TEST_F(HoughLinesProcessorTest, Process_ArrowStartsEndsHaveSameSize) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_EQ(out.arrowStarts.size(), out.arrowEnds.size());
}

TEST_F(HoughLinesProcessorTest, Process_KeypointsAndOverlayAreEmpty) {
    auto out = proc.process(TestImage::checkerboard(W, H));
    EXPECT_TRUE(out.keypoints.empty());
    EXPECT_TRUE(out.overlayImage.empty());
}

TEST_F(HoughLinesProcessorTest, SetParameter_HighThreshold_ReducesLines) {
    proc.setParameter("threshold", 10.f);
    auto more = proc.process(TestImage::checkerboard(W, H)).arrowStarts.size();

    proc.setParameter("threshold", 200.f);
    auto fewer = proc.process(TestImage::checkerboard(W, H)).arrowStarts.size();

    EXPECT_GE(more, fewer);
}

TEST_F(HoughLinesProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("unknown", 0.f));
    EXPECT_NO_THROW(proc.process(TestImage::checkerboard(W, H)));
}

// ─────────────────────────────────────────────────────────────────────────────
// HoughCirclesProcessor
// ─────────────────────────────────────────────────────────────────────────────

class HoughCirclesProcessorTest : public ::testing::Test {
protected:
    HoughCirclesProcessor proc;
    static constexpr int W = 320, H = 240;
};

TEST_F(HoughCirclesProcessorTest, Name_IsHoughCircles) {
    EXPECT_EQ("HoughCircles", proc.name());
}

TEST_F(HoughCirclesProcessorTest, OutputType_IsKEYPOINTS) {
    EXPECT_EQ(OutputType::KEYPOINTS, proc.outputType());
}

TEST_F(HoughCirclesProcessorTest, ParamDefs_HasRequiredKeys) {
    auto defs = proc.paramDefs();
    auto has = [&](const std::string& k) {
        return std::any_of(defs.begin(), defs.end(),
            [&](const ParamDef& d){ return d.key == k; });
    };
    EXPECT_TRUE(has("dp"));
    EXPECT_TRUE(has("minDist"));
    EXPECT_TRUE(has("param1"));
    EXPECT_TRUE(has("param2"));
    EXPECT_TRUE(has("minRadius"));
    EXPECT_TRUE(has("maxRadius"));
}

TEST_F(HoughCirclesProcessorTest, ParamDefs_AllRangesValid) {
    for (const auto& d : proc.paramDefs())
        EXPECT_LT(d.min, d.max) << "param " << d.key << " has invalid range";
}

TEST_F(HoughCirclesProcessorTest, ParamDefs_DefaultsInRange) {
    for (const auto& d : proc.paramDefs())
        EXPECT_TRUE(d.defaultValue >= d.min && d.defaultValue <= d.max)
            << "param " << d.key << " default out of range";
}

TEST_F(HoughCirclesProcessorTest, Process_CircledImage_DetectsCircle) {
    proc.setParameter("param2", 15.f);
    proc.setParameter("minRadius", 40.f);
    proc.setParameter("maxRadius", 80.f);
    auto out = proc.process(TestImage::circled(W, H));
    EXPECT_GT(out.keypoints.size(), 0u);
}

TEST_F(HoughCirclesProcessorTest, Process_UniformImage_NoCircles) {
    auto out = proc.process(TestImage::uniform(W, H));
    EXPECT_EQ(out.keypoints.size(), 0u);
}

TEST_F(HoughCirclesProcessorTest, Process_KeypointSize_IsPositive) {
    proc.setParameter("param2", 15.f);
    proc.setParameter("minRadius", 40.f);
    proc.setParameter("maxRadius", 80.f);
    auto out = proc.process(TestImage::circled(W, H));
    for (const auto& kp : out.keypoints)
        EXPECT_GT(kp.size, 0.f);
}

TEST_F(HoughCirclesProcessorTest, Process_ArrowVectorsAndOverlayAreEmpty) {
    auto out = proc.process(TestImage::circled(W, H));
    EXPECT_TRUE(out.arrowStarts.empty());
    EXPECT_TRUE(out.overlayImage.empty());
}

TEST_F(HoughCirclesProcessorTest, SetParameter_UnknownKey_DoesNotCrash) {
    EXPECT_NO_THROW(proc.setParameter("unknown", 0.f));
    EXPECT_NO_THROW(proc.process(TestImage::circled(W, H)));
}
