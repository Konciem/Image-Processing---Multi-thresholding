#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "raylib.h"
#include <vector>

void ImageMultiThreshold(Image *image, const std::vector<int>& thresholds, const std::vector<Color>& colors);
void ImageSingleThreshold_ASM(Image *image, unsigned char threshold, Color colorBelow, Color colorAbove);

#endif
