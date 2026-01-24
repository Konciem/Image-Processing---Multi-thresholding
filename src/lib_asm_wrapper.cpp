#include "common.hpp"

extern "C" void ImageSingleThreshold_NEON(
    unsigned int* pixels, 
    long pixelCount, 
    unsigned char threshold, 
    unsigned char* colorPtr 
);

extern "C" {
    void ProcessPart(unsigned int* pixels, int pixelCount, 
                     int* thresholds, int thresholdCount, 
                     unsigned int* colors, int colorCount) {

        if (thresholdCount == 1 && colorCount == 2) {
            unsigned char* rawColors = (unsigned char*)colors;
            int limit = pixelCount - (pixelCount % 16);

            if (limit > 0) {
                ImageSingleThreshold_NEON(pixels, limit, (unsigned char)thresholds[0], rawColors);
            }

            unsigned char* byteData = (unsigned char*)pixels;
            unsigned char* cBelow = &rawColors[0];
            unsigned char* cAbove = &rawColors[4];

            for (int i = limit; i < pixelCount; i++) {
                 unsigned char val = byteData[i * 4];
                 unsigned char* targetColor = (val >= thresholds[0]) ? cAbove : cBelow;
                 
                 byteData[i*4+0] = targetColor[0];
                 byteData[i*4+1] = targetColor[1];
                 byteData[i*4+2] = targetColor[2];
                 byteData[i*4+3] = targetColor[3];
            }
            return;
        }

        if (colorCount != thresholdCount + 1) return;

        unsigned char* byteData = (unsigned char*)pixels;
        unsigned char* colorBytes = (unsigned char*)colors; 

        for (int i = 0; i < pixelCount; i++) {
            unsigned char grayVal = byteData[i * 4]; 
            int colorIndex = 0;
            while (colorIndex < thresholdCount && grayVal >= thresholds[colorIndex]) {
                colorIndex++;
            }
            int offset = colorIndex * 4;
            byteData[i * 4 + 0] = colorBytes[offset + 0];
            byteData[i * 4 + 1] = colorBytes[offset + 1];
            byteData[i * 4 + 2] = colorBytes[offset + 2];
            byteData[i * 4 + 3] = colorBytes[offset + 3];
        }
    }
}