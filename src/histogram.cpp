#include "histogram.hpp"
#include <algorithm> 
#include <string>
#include <map>

std::vector<int> GenerateHistogram(const Image& grayscaleImage) {
    std::vector<int> histogram(256, 0);
    Color* pixels = LoadImageColors(grayscaleImage);

    for (int i = 0; i < grayscaleImage.width * grayscaleImage.height; i++) {
        histogram[pixels[i].r]++;
    }

    UnloadImageColors(pixels);
    return histogram;
}

void DrawHistogram(const std::vector<int>& histogramData, Rectangle bounds, const std::vector<int>& thresholds) {
    const float paddingX = 30;
    const float paddingY = 20;
    Rectangle chartBounds = { bounds.x + paddingX, bounds.y, bounds.width - paddingX, bounds.height - paddingY };

    DrawText("0", chartBounds.x, chartBounds.y + chartBounds.height + 5, 10, DARKGRAY);
    DrawText("255", chartBounds.x + chartBounds.width - MeasureText("255", 10), chartBounds.y + chartBounds.height + 5, 10, DARKGRAY);

    if (histogramData.empty()) {
        DrawRectangleLinesEx(bounds, 1, DARKGRAY);
        DrawText("0", bounds.x, bounds.y + bounds.height - paddingY, 10, DARKGRAY);
        return;
    }

    int maxCount = *std::max_element(histogramData.begin(), histogramData.end());
    DrawText(std::to_string(maxCount).c_str(), bounds.x, bounds.y, 10, DARKGRAY);
    DrawText("0", bounds.x, bounds.y + bounds.height - paddingY, 10, DARKGRAY);

    if (maxCount == 0) {
        DrawRectangleLinesEx(chartBounds, 1, DARKGRAY);
        return;
    }

    float barWidth = chartBounds.width / 256.0f;

    for (int i = 0; i < 256; i++) {
        float barHeight = ((float)histogramData[i] / maxCount) * chartBounds.height;
        Rectangle bar = {
            chartBounds.x + i * barWidth,
            chartBounds.y + chartBounds.height - barHeight,
            barWidth,
            barHeight
        };
        DrawRectangleRec(bar, Color{(unsigned char)i, (unsigned char)i, (unsigned char)i, 255});
    }

    for (int threshold : thresholds) {
        if (threshold >= 0 && threshold < 256) {
            float thresholdX = chartBounds.x + threshold * barWidth;
            DrawLineV({thresholdX, chartBounds.y}, {thresholdX, chartBounds.y + chartBounds.height}, MAROON);
        }
    }

    DrawRectangleLinesEx(chartBounds, 1, DARKGRAY);
}

std::vector<int> GenerateColorHistogram(const Image& image, const std::vector<Color>& palette) {
    if (palette.empty()) return {};

    std::vector<int> histogram(palette.size(), 0);
    std::map<unsigned int, int> colorToIndex;
    for (size_t i = 0; i < palette.size(); ++i) {
        unsigned int colorKey = (palette[i].r << 24) | (palette[i].g << 16) | (palette[i].b << 8) | palette[i].a;
        colorToIndex[colorKey] = i;
    }

    Color* pixels = LoadImageColors(image);
    for (int i = 0; i < image.width * image.height; i++) {
        unsigned int pixelKey = (pixels[i].r << 24) | (pixels[i].g << 16) | (pixels[i].b << 8) | pixels[i].a;
        auto it = colorToIndex.find(pixelKey);
        if (it != colorToIndex.end()) {
            histogram[it->second]++;
        }
    }

    UnloadImageColors(pixels);
    return histogram;
}

void DrawColorHistogram(const std::vector<int>& histogramData, const std::vector<Color>& palette, Rectangle bounds) {
    const float paddingX = 30;
    const float paddingY = 20;
    Rectangle chartBounds = { bounds.x + paddingX, bounds.y, bounds.width - paddingX, bounds.height - paddingY };

    if (histogramData.empty() || palette.empty()) {
        DrawRectangleLinesEx(bounds, 1, DARKGRAY);
        DrawText("0", bounds.x, bounds.y + bounds.height - paddingY, 10, DARKGRAY);
        return;
    }

    int maxCount = *std::max_element(histogramData.begin(), histogramData.end());
    DrawText(std::to_string(maxCount).c_str(), bounds.x, bounds.y, 10, DARKGRAY);
    DrawText("0", bounds.x, bounds.y + bounds.height - paddingY, 10, DARKGRAY);

    if (maxCount == 0) {
        DrawRectangleLinesEx(chartBounds, 1, DARKGRAY);
        return;
    }

    float barWidth = chartBounds.width / histogramData.size();

    for (size_t i = 0; i < histogramData.size(); i++) {
        float barHeight = ((float)histogramData[i] / maxCount) * chartBounds.height;
        Rectangle bar = {
            chartBounds.x + i * barWidth,
            chartBounds.y + chartBounds.height - barHeight,
            barWidth,
            barHeight
        };
        DrawRectangleRec(bar, palette[i]);

        Rectangle swatch = { chartBounds.x + i * barWidth + barWidth/2 - 5, chartBounds.y + chartBounds.height + 5, 10, 10 };
        DrawRectangleRec(swatch, palette[i]);
        DrawRectangleLinesEx(swatch, 1, DARKGRAY);
    }
    DrawRectangleLinesEx(chartBounds, 1, DARKGRAY);
}

int GetThresholdFromMouse(Vector2 mousePos, Rectangle bounds) {
    const float paddingX = 30;
    const float paddingY = 20;
    Rectangle chartBounds = { bounds.x + paddingX, bounds.y, bounds.width - paddingX, bounds.height - paddingY };

    if (CheckCollisionPointRec(mousePos, chartBounds)) {
        float barWidth = chartBounds.width / 256.0f;
        int threshold = (int)((mousePos.x - chartBounds.x) / barWidth);

        if (threshold < 0) {
            threshold = 0;
        }
        if (threshold > 255) {
            threshold = 255;
        }
        return threshold;
    }
    return -1; 
}