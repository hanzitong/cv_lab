#pragma once

#include <opencv2/core.hpp>
#include <cstdint>

namespace TestImage {

// チェッカーボード（squareSizeのマス）— コーナーが豊富で検出器が確実に反応
inline cv::Mat checkerboard(int w = 320, int h = 240, int squareSize = 32) {
    cv::Mat img(h, w, CV_8UC1);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            img.at<uint8_t>(y, x) =
                ((x / squareSize + y / squareSize) % 2 == 0) ? 0u : 255u;
    return img;
}

// 均一グレー — 特徴点なし
inline cv::Mat uniform(int w = 320, int h = 240, uint8_t value = 128) {
    return cv::Mat(h, w, CV_8UC1, cv::Scalar(value));
}

// 横グラデーション — 緩やかなエッジ
inline cv::Mat gradient(int w = 320, int h = 240) {
    cv::Mat img(h, w, CV_8UC1);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            img.at<uint8_t>(y, x) = static_cast<uint8_t>(x * 255 / w);
    return img;
}

// ランダムノイズ — 多数の弱い特徴点
inline cv::Mat noise(int w = 320, int h = 240) {
    cv::Mat img(h, w, CV_8UC1);
    cv::randu(img, 0, 256);
    return img;
}

// 合成ドット（明確なブロブ）— 輝点が散在
inline cv::Mat dots(int w = 320, int h = 240, int spacing = 40) {
    cv::Mat img = uniform(w, h, 50);
    for (int y = spacing / 2; y < h; y += spacing)
        for (int x = spacing / 2; x < w; x += spacing)
            cv::circle(img, {x, y}, 6, cv::Scalar(200), -1);
    return img;
}

} // namespace TestImage
