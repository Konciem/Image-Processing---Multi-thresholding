#ifndef COMMON_HPP
#define COMMON_HPP

#include "raylib.h"

typedef void (*ProcessingFunc)(unsigned int* pixels, int pixelCount, 
                               int* thresholds, int thresholdCount, 
                               unsigned int* colors, int colorCount);

#endif