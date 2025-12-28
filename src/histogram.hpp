#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

#include "raylib.h"
#include <vector>

std::vector<int> GenerateHistogram(const Image& grayscaleImage);

void DrawHistogram(const std::vector<int>& histogramData, Rectangle bounds, const std::vector<int>& thresholds);

std::vector<int> GenerateColorHistogram(const Image& image, const std::vector<Color>& palette);

void DrawColorHistogram(const std::vector<int>& histogramData, const std::vector<Color>& palette, Rectangle bounds);

int GetThresholdFromMouse(Vector2 mousePos, Rectangle bounds);

#endif // HISTOGRAM_HPP