#include "image_processing.hpp"

void ImageMultiThreshold(Image *image, const std::vector<int>& thresholds, const std::vector<Color>& colors) {
    if (thresholds.size() + 1 != colors.size()) return;

    Color *pixels = (Color *)image->data;

    for (int i = 0; i < image->width * image->height; i++) {
        unsigned char grayValue = pixels[i].r;

        size_t colorIndex = 0;

        while (colorIndex < thresholds.size() && grayValue >= thresholds[colorIndex]) {
            colorIndex++;
        }

        pixels[i] = colors[colorIndex];
    }
}

extern "C" void ImageSingleThreshold_NEON(unsigned int* pixels, long pixelCount, unsigned char threshold, unsigned int colorBelow, unsigned int colorAbove);

void ImageSingleThreshold_ASM(Image *image, unsigned char threshold, Color colorBelow, Color colorAbove) {
    if (image->format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        return;
    }

    unsigned char *pixels = (unsigned char *)image->data;
    int pixelCount = image->width * image->height;

    unsigned int below = (colorBelow.a << 24) | (colorBelow.b << 16) | (colorBelow.g << 8) | colorBelow.r;
    unsigned int above = (colorAbove.a << 24) | (colorAbove.b << 16) | (colorAbove.g << 8) | colorAbove.r;

    unsigned int* pixels_u32 = (unsigned int*)pixels;
    int limit = pixelCount - (pixelCount % 16);

    if (limit > 0) {
        ImageSingleThreshold_NEON(pixels_u32, limit, threshold, below, above);
    }

    for (int i = limit; i < pixelCount; ++i) {
        if (pixels[i * 4] >= threshold) pixels_u32[i] = above;
        else pixels_u32[i] = below;
    }
}
