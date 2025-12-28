#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

#include "raylib.h"
#include <vector>

// Generuje dane histogramu z obrazu w skali szarości.
// Histogram będzie miał 256 wartości, po jednej dla każdego poziomu szarości.
std::vector<int> GenerateHistogram(const Image& grayscaleImage);

// Rysuje histogram wewnątrz podanego prostokąta.
void DrawHistogram(const std::vector<int>& histogramData, Rectangle bounds, const std::vector<int>& thresholds);

// Generuje dane histogramu na podstawie palety kolorów.
// Zlicza, ile pikseli w obrazie odpowiada każdemu kolorowi z palety.
std::vector<int> GenerateColorHistogram(const Image& image, const std::vector<Color>& palette);

// Rysuje histogram kolorów.
void DrawColorHistogram(const std::vector<int>& histogramData, const std::vector<Color>& palette, Rectangle bounds);

// Oblicza wartość progu (0-255) na podstawie pozycji myszy na histogramie.
// Zwraca -1, jeśli mysz jest poza obszarem wykresu.
int GetThresholdFromMouse(Vector2 mousePos, Rectangle bounds);

#endif // HISTOGRAM_HPP