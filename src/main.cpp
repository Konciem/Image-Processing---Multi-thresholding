#include "raylib.h"
#include "image_processing.hpp"
#include "histogram.hpp"

#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

int main() {
    const int screenWidth = 1180;
    const int screenHeight = 800;
    
    InitWindow(screenWidth, screenHeight, "Progowanie Obrazu");

    Font customFont = LoadFontEx("resources/anonymous_pro_bold.ttf", 40, 0, 250);

    const float padding = 20.0f;
    const float topMargin = 40.0f;
    const float panelHeight = 120.0f; 

    const float zoneWidth = (screenWidth - 3 * padding) / 2.0f;
    const float zoneHeight = (screenHeight - 3 * padding - topMargin - panelHeight) / 2.0f;

    Rectangle dropZone = { padding, padding + topMargin, zoneWidth, zoneHeight };
    Rectangle displayZoneOriginal = { padding * 2 + zoneWidth, padding + topMargin, zoneWidth, zoneHeight };

    Rectangle displayZoneGrayscale = { padding, padding * 2 + zoneHeight + topMargin, zoneWidth, zoneHeight };
    Rectangle displayZoneMultiThreshold = { padding * 2 + zoneWidth, padding * 2 + topMargin + zoneHeight, zoneWidth, zoneHeight };

    Rectangle colorPickerPanel = { padding, screenHeight - panelHeight, screenWidth - 2 * padding, panelHeight - padding };
    vector<Rectangle> colorSwatches;
    int selectedColorIndex = 0;
    bool imageNeedsUpdate = false;

    vector<int> thresholds = {80, 160};
    vector<Color> multiColors = {BLACK, GRAY, WHITE};

    int draggingThresholdIndex = -1; 

    Texture2D originalTexture = { 0 }, grayscaleTexture = { 0 }, multiThresholdTexture = { 0 };
    Image grayscaleImage = { 0 }; 
    vector<int> multiThresholdHistogramData;
    bool imageLoaded = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if (imageLoaded) {
            if (CheckCollisionPointRec(GetMousePosition(), dropZone)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    int newThreshold = GetThresholdFromMouse(GetMousePosition(), dropZone);
                    if (newThreshold != -1) {

                        for (size_t i = 0; i < thresholds.size(); ++i) {
                            if (abs(newThreshold - thresholds[i]) < 5) { 
                                draggingThresholdIndex = i;
                                break;
                            }
                        }

                        if (draggingThresholdIndex == -1 && IsKeyDown(KEY_LEFT_SHIFT)) {
                            thresholds.push_back(newThreshold);
                            sort(thresholds.begin(), thresholds.end());
                            imageNeedsUpdate = true;
                        }

                        else if (draggingThresholdIndex != -1 && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER))) {
                             thresholds.erase(thresholds.begin() + draggingThresholdIndex);
                             draggingThresholdIndex = -1;
                             imageNeedsUpdate = true;
                        }
                    }
                }
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                draggingThresholdIndex = -1;
            }
        }

        // logika ui
        if (imageLoaded) {
            for (size_t i = 0; i < colorSwatches.size(); i++) {
                if (i < colorSwatches.size() && CheckCollisionPointRec(GetMousePosition(), colorSwatches[i]) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selectedColorIndex = i;
                }
            }
        }

        // Drag & Drop
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (CheckCollisionPointRec(GetMousePosition(), dropZone)) {
                if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".png")) {

                    if (imageLoaded) {
                        UnloadTexture(originalTexture);
                        UnloadTexture(grayscaleTexture);
                        UnloadTexture(multiThresholdTexture);
                    }

                    if (grayscaleImage.data) {
                        UnloadImage(grayscaleImage);
                    }

                    Image originalImage = LoadImage(droppedFiles.paths[0]); 
                    grayscaleImage = ImageCopy(originalImage);             
                    ImageColorGrayscale(&grayscaleImage);

                    originalTexture = LoadTextureFromImage(originalImage);
                    grayscaleTexture = LoadTextureFromImage(grayscaleImage);

                    UnloadImage(originalImage); 
                    imageLoaded = true;
                    imageNeedsUpdate = true; 
                }
            }

            UnloadDroppedFiles(droppedFiles);
        }

        if (imageLoaded && draggingThresholdIndex != -1) {
            int newThresholdValue = GetThresholdFromMouse(GetMousePosition(), dropZone);
            if (newThresholdValue != -1) {
                thresholds[draggingThresholdIndex] = newThresholdValue;
                sort(thresholds.begin(), thresholds.end());
                imageNeedsUpdate = true;
            }
        }

        if (imageLoaded && imageNeedsUpdate) {
            if (multiThresholdTexture.id > 0) {
                UnloadTexture(multiThresholdTexture);
            }

            multiColors.resize(thresholds.size() + 1, Color{(unsigned char)GetRandomValue(0,255), (unsigned char)GetRandomValue(0,255), (unsigned char)GetRandomValue(0,255), 255});

            Image multiThresholdImage = ImageCopy(grayscaleImage); 
            ImageFormat(&multiThresholdImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            ImageMultiThreshold(&multiThresholdImage, thresholds, multiColors);

            multiThresholdTexture = LoadTextureFromImage(multiThresholdImage);

            multiThresholdHistogramData = GenerateColorHistogram(multiThresholdImage, multiColors);
            UnloadImage(multiThresholdImage); 

            imageNeedsUpdate = false;
        }


        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Histogram (skala szarosci) | SHIFT+Click: dodaj, CTRL+Click: usun", dropZone.x, dropZone.y - 25, 10, DARKGRAY);
        if (imageLoaded) {
            DrawHistogram(GenerateHistogram(grayscaleImage), dropZone, thresholds);
            for(int threshold : thresholds) {
                float thresholdX = dropZone.x + 30 + (float)threshold / 255.0f * (dropZone.width - 30);
                DrawText(to_string(threshold).c_str(), thresholdX - 10, dropZone.y + 5, 10, MAROON);
            }
        } else {
            DrawText("Strefa upuszczania", dropZone.x, dropZone.y - 15, 10, DARKGRAY);
            DrawRectangleLinesEx(dropZone, 2, GRAY);
            DrawText("Upusc obraz .png", dropZone.x + (dropZone.width - MeasureText("Upuść .png", 20))/2, dropZone.y + zoneHeight/2 -10, 20, GRAY);
        }
        DrawText("Oryginal", displayZoneOriginal.x, displayZoneOriginal.y - 15, 10, DARKGRAY);
        DrawRectangleLinesEx(displayZoneOriginal, 2, LIGHTGRAY);

        DrawText("Skala szarosci", displayZoneGrayscale.x, displayZoneGrayscale.y - 15, 10, DARKGRAY);
        DrawRectangleLinesEx(displayZoneGrayscale, 2, LIGHTGRAY);

        DrawText("Wieloprogowanie", displayZoneMultiThreshold.x, displayZoneMultiThreshold.y - 15, 10, DARKGRAY);
        DrawRectangleLinesEx(displayZoneMultiThreshold, 2, LIGHTGRAY);

        if (imageLoaded) {
            auto drawTextureInZone = [](Texture2D texture, Rectangle zone) {
                float scale = min(zone.width / texture.width, zone.height / texture.height);
                float newWidth = texture.width * scale;
                float newHeight = texture.height * scale;

                float x = zone.x + (zone.width - newWidth) / 2;
                float y = zone.y + (zone.height - newHeight) / 2;

                DrawTexturePro(texture,
                               { 0, 0, (float)texture.width, (float)texture.height },
                               { x, y, newWidth, newHeight },
                               { 0, 0 }, 0.0f, WHITE);
            };

            drawTextureInZone(originalTexture, displayZoneOriginal);
            drawTextureInZone(grayscaleTexture, displayZoneGrayscale);
            drawTextureInZone(multiThresholdTexture, displayZoneMultiThreshold);
        } else {
            DrawText("Podglad", displayZoneOriginal.x + (displayZoneOriginal.width - MeasureText("Podgląd", 20))/2, displayZoneOriginal.y + zoneHeight/2 -10, 20, LIGHTGRAY);
            DrawText("Podglad", displayZoneGrayscale.x + (displayZoneGrayscale.width - MeasureText("Podgląd", 20))/2, displayZoneGrayscale.y + zoneHeight/2 -10, 20, LIGHTGRAY);
            DrawText("Podglad", displayZoneMultiThreshold.x + (displayZoneMultiThreshold.width - MeasureText("Podgląd", 20))/2, displayZoneMultiThreshold.y + zoneHeight/2 -10, 20, LIGHTGRAY);
        }

        DrawRectangleRec(colorPickerPanel, Fade(LIGHTGRAY, 0.5f));
        DrawRectangleLinesEx(colorPickerPanel, 1, DARKGRAY);
        DrawText("Edytor Palety", colorPickerPanel.x + 10, colorPickerPanel.y + 10, 20, DARKGRAY);

        if (imageLoaded) {
            float swatchSize = 60;
            colorSwatches.resize(multiColors.size());
            for (size_t i = 0; i < multiColors.size(); i++) {
                colorSwatches[i] = { colorPickerPanel.x + 190 + (swatchSize + 10) * (float)i, colorPickerPanel.y + 20, swatchSize, swatchSize };
                DrawRectangleRec(colorSwatches[i], multiColors[i]);
                if (i == (size_t)selectedColorIndex) {
                    DrawRectangleLinesEx(colorSwatches[i], 3, MAROON);
                } else {
                    DrawRectangleLinesEx(colorSwatches[i], 1, GRAY);
                }
            }

            if (selectedColorIndex < (int)multiColors.size()) {
                float sliderWidth = 300;
                float sliderHeight = 20;
                Rectangle rSlider = { colorPickerPanel.x + 190 + (swatchSize + 10) * (float)multiColors.size() + 20, colorPickerPanel.y + 15, sliderWidth, sliderHeight };
                Rectangle gSlider = { rSlider.x, colorPickerPanel.y + 40, sliderWidth, sliderHeight };
                Rectangle bSlider = { rSlider.x, colorPickerPanel.y + 65, sliderWidth, sliderHeight };

                unsigned char* channels[] = { &multiColors[selectedColorIndex].r, &multiColors[selectedColorIndex].g, &multiColors[selectedColorIndex].b };
                Rectangle sliders[] = { rSlider, gSlider, bSlider };
                const char* labels[] = { "R", "G", "B" };
                Color sliderColors[] = { RED, GREEN, BLUE };

                for (int i = 0; i < 3; i++) {
                    DrawText(labels[i], sliders[i].x - 20, sliders[i].y + 5, 10, DARKGRAY);
                    DrawRectangleRec(sliders[i], LIGHTGRAY);
                    DrawRectangleRec({ sliders[i].x, sliders[i].y, sliders[i].width * (*channels[i] / 255.0f), sliders[i].height }, sliderColors[i]);
                    DrawRectangleLinesEx(sliders[i], 1, GRAY);

                    if (CheckCollisionPointRec(GetMousePosition(), sliders[i]) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                        float newValue = ((GetMousePosition().x - sliders[i].x) / sliders[i].width) * 255.0f;
                        *channels[i] = (unsigned char)clamp(newValue, 0.0f, 255.0f);
                        imageNeedsUpdate = true;
                    }
                }
            }
        } else {
            DrawText("Wczytaj obraz, aby edytowac palete...", colorPickerPanel.x + 150, colorPickerPanel.y + 45, 20, GRAY);
        }

        DrawFPS(10, 10);
        DrawText("MultiThreshold", screenWidth/2 - MeasureText("MultiThreshold", 20)/2, 10, 20, GRAY);
        EndDrawing();
    }

    UnloadFont(customFont);

    if (imageLoaded) {
        UnloadTexture(originalTexture);
        UnloadTexture(grayscaleTexture);
        UnloadTexture(multiThresholdTexture);
        multiThresholdHistogramData.clear();
    }

    CloseWindow();
    return 0;
}
