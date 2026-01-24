#include "common.hpp"
#include <vector>

extern "C" {

    void ProcessPart(unsigned int* pixels, int pixelCount, 
                     int* thresholds, int thresholdCount, 
                     unsigned int* colors, int colorCount) {
        
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
            
            byteData[i * 4 + 0] = colorBytes[offset + 0]; // R
            byteData[i * 4 + 1] = colorBytes[offset + 1]; // G
            byteData[i * 4 + 2] = colorBytes[offset + 2]; // B
            byteData[i * 4 + 3] = colorBytes[offset + 3]; // A
        }
    }
}