#include "raylib.h"
#include "histogram.hpp"
#include "common.hpp"

#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <dlfcn.h> 
#include <cstring> 
#include <cmath> 
#include <fstream> 

using namespace std;

struct BenchmarkResult {
    string libName;
    int threads;
    long long timeUs;
};


vector<BenchmarkResult> benchmarkResults;

const Color COL_BG          = { 30, 30, 35, 255 };
const Color COL_PANEL       = { 45, 45, 50, 255 };
const Color COL_ACCENT      = { 60, 130, 240, 255 };
const Color COL_ACCENT_HOVER= { 80, 150, 255, 255 };
const Color COL_TEXT        = { 220, 220, 220, 255 };
const Color COL_TEXT_MUTED  = { 150, 150, 150, 255 };
const Color COL_DANGER      = { 220, 60, 60, 255 };
const Color COL_HIGHLIGHT   = { 255, 255, 0, 255 }; 
const Color COL_SUCCESS     = { 50, 200, 50, 255 }; 

void* libHandle = nullptr;
ProcessingFunc processFunc = nullptr;
string currentLibName = "Brak";

bool showSaveMessage = false;
int saveMessageCounter = 0;
bool showAddMessage = false;
int addMessageCounter = 0;


bool LoadLibraryAlgorithm(const char* libPath) {
    if (libHandle) { dlclose(libHandle); libHandle = nullptr; processFunc = nullptr; }
    libHandle = dlopen(libPath, RTLD_LAZY);
    if (!libHandle) return false;
    processFunc = (ProcessingFunc)dlsym(libHandle, "ProcessPart");
    return (processFunc != nullptr);
}

bool DrawStyledButton(Rectangle rect, const char* text, bool isActive = false, Color customColor = COL_ACCENT) {
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    Color bg = isActive ? customColor : (hovered ? Color{60,60,65,255} : Color{50,50,55,255});
    if (isActive && hovered) bg = Color{ (unsigned char)(customColor.r+20), (unsigned char)(customColor.g+20), (unsigned char)(customColor.b+20), 255 };

    DrawRectangleRec(rect, bg);
    DrawRectangleLinesEx(rect, 1, isActive ? COL_TEXT : Color{80,80,80,255});
    
    int textW = MeasureText(text, 20);
    DrawText(text, rect.x + (rect.width - textW)/2, rect.y + (rect.height - 20)/2, 20, COL_TEXT);
    
    return clicked;
}

unsigned int ColorToUInt(Color c) {
    return (unsigned int)((c.a << 24) | (c.b << 16) | (c.g << 8) | c.r);
}

Color GetRandomColor() {
    return { (unsigned char)GetRandomValue(50, 255), (unsigned char)GetRandomValue(50, 255), (unsigned char)GetRandomValue(50, 255), 255 };
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 900; 
    SetConfigFlags(FLAG_MSAA_4X_HINT); 
    InitWindow(screenWidth, screenHeight, "JA Projekt - Progowanie (ARM64)");

    if(LoadLibraryAlgorithm("./libalgo_cpp.dylib")) currentLibName = "C++";

    Texture2D originalTexture = {0}, resultTexture = {0};
    Image originalImage = {0}, resultImage = {0};
    bool imageLoaded = false;
    bool needsUpdate = false;
    
    vector<int> thresholds = { 128 };
    vector<Color> segmentColors = { BLACK, WHITE };
    int draggingThresholdIdx = -1;
    
    bool useASM = false;
    int threadCount = 4;
    long long executionTime = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        // --- LOGIKA PLIKÓW ---
        if (IsFileDropped()) {
            FilePathList dropped = LoadDroppedFiles();
            if (dropped.count > 0 && IsFileExtension(dropped.paths[0], ".png")) {
                if (imageLoaded) {
                    UnloadTexture(originalTexture); UnloadTexture(resultTexture);
                    UnloadImage(originalImage); UnloadImage(resultImage);
                }

                originalImage = LoadImage(dropped.paths[0]);
                ImageFormat(&originalImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
                ImageColorGrayscale(&originalImage); 
                ImageFormat(&originalImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

                originalTexture = LoadTextureFromImage(originalImage);
                resultImage = ImageCopy(originalImage);
                resultTexture = LoadTextureFromImage(resultImage);
                
                imageLoaded = true;
                needsUpdate = true;
                executionTime = 0;
            }
            UnloadDroppedFiles(dropped);
        }

        // --- OBSŁUGA MYSZY (Histogram) ---
        Rectangle histogramRect = { 340, screenHeight - 140.0f, screenWidth - 360.0f, 100.0f };
        Rectangle interactionRect = { histogramRect.x - 10, histogramRect.y - 20, histogramRect.width + 20, histogramRect.height + 40 };
        int hoveredThresholdIdx = -1;

        if (imageLoaded) {
            Vector2 mouse = GetMousePosition();
            
            if (CheckCollisionPointRec(mouse, interactionRect)) {
                float minDist = 15.0f;
                for (size_t i = 0; i < thresholds.size(); i++) {
                    float thX = histogramRect.x + ((float)thresholds[i] / 255.0f) * histogramRect.width;
                    float dist = abs(mouse.x - thX);
                    if (dist < minDist) {
                        minDist = dist;
                        hoveredThresholdIdx = (int)i;
                    }
                }

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    float normalizedX = (mouse.x - histogramRect.x) / histogramRect.width;
                    int newTh = (int)(normalizedX * 255.0f);
                    if (newTh < 0) newTh = 0; if (newTh > 255) newTh = 255;

                    if (hoveredThresholdIdx != -1) {
                        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER)) {
                            thresholds.erase(thresholds.begin() + hoveredThresholdIdx);
                            if (segmentColors.size() > 2) segmentColors.pop_back();
                            hoveredThresholdIdx = -1;
                            needsUpdate = true;
                        } else {
                            draggingThresholdIdx = hoveredThresholdIdx;
                        }
                    } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        thresholds.push_back(newTh);
                        sort(thresholds.begin(), thresholds.end());
                        segmentColors.push_back(GetRandomColor());
                        needsUpdate = true;
                    }
                }
            }

            if (draggingThresholdIdx != -1) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    float normalizedX = (mouse.x - histogramRect.x) / histogramRect.width;
                    int val = (int)(normalizedX * 255.0f);
                    if (val < 0) val = 0; if (val > 255) val = 255;
                    thresholds[draggingThresholdIdx] = val;
                    sort(thresholds.begin(), thresholds.end());
                    needsUpdate = true;
                } else {
                    draggingThresholdIdx = -1;
                }
            }
        }

        // --- PRZETWARZANIE ---
        if (imageLoaded && needsUpdate && processFunc) {
            memcpy(resultImage.data, originalImage.data, originalImage.width * originalImage.height * 4);
            unsigned int* pixels = (unsigned int*)resultImage.data;
            int totalPixels = resultImage.width * resultImage.height;

            vector<unsigned int> rawColors;
            for(auto& c : segmentColors) c.a = 255; 
            for(auto c : segmentColors) rawColors.push_back(ColorToUInt(c));
            while(rawColors.size() < thresholds.size() + 1) rawColors.push_back(ColorToUInt(WHITE));
            while(rawColors.size() > thresholds.size() + 1) rawColors.pop_back();

            auto start = std::chrono::high_resolution_clock::now();
            vector<std::thread> threads;
            int tCount = (threadCount < 1) ? 1 : threadCount;
            int pxPerThread = totalPixels / tCount;

            for(int i=0; i<tCount; i++) {
                int startPx = i * pxPerThread;
                int count = (i == tCount-1) ? (totalPixels - startPx) : pxPerThread;
                threads.emplace_back(processFunc, pixels + startPx, count, thresholds.data(), (int)thresholds.size(), rawColors.data(), (int)rawColors.size());
            }

            for(auto& t : threads) t.join();
            auto end = std::chrono::high_resolution_clock::now();
            executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            
            UpdateTexture(resultTexture, resultImage.data);
            needsUpdate = false;
        }

        // --- RYSOWANIE ---
        BeginDrawing();
        ClearBackground(COL_BG);

        // Sidebar
        Rectangle sidebar = { 0, 0, 300, (float)screenHeight };
        DrawRectangleRec(sidebar, COL_PANEL);
        DrawRectangleLinesEx({sidebar.width-1, 0, 1, sidebar.height}, 1, Color{60,60,60,255});
        DrawText("JA PROJEKT", 20, 30, 30, COL_TEXT);
        DrawText("Progowanie Obrazu", 20, 65, 20, COL_TEXT_MUTED);

        float y = 120;
        DrawText("Biblioteka:", 20, y, 18, COL_TEXT_MUTED);
        y += 25;
        if (DrawStyledButton({ 20, y, 125, 40 }, "C++", !useASM)) { if (LoadLibraryAlgorithm("./libalgo_cpp.dylib")) { useASM = false; currentLibName="C++"; needsUpdate=true; } }
        if (DrawStyledButton({ 155, y, 125, 40 }, "ASM", useASM)) { if (LoadLibraryAlgorithm("./libalgo_asm.dylib")) { useASM = true; currentLibName="ASM"; needsUpdate=true; } }
        y += 60;

        DrawText(TextFormat("Liczba watkow: %d", threadCount), 20, y, 18, COL_TEXT_MUTED);
        y += 25;
        if (DrawStyledButton({20, y, 40, 30}, "-")) { if(threadCount > 1) {threadCount--; needsUpdate=true;} }
        DrawRectangleLinesEx({70, y, 160, 30}, 1, COL_TEXT_MUTED);
        DrawRectangleRec({70, y, (float)((threadCount/64.0f)*160), 30}, COL_ACCENT); 
        if (DrawStyledButton({240, y, 40, 30}, "+")) { if(threadCount < 64) {threadCount++; needsUpdate=true;} }
        y += 50;

        DrawRectangleRec({20, y, 260, 80}, Color{40,40,40,255});
        DrawRectangleLinesEx({20, y, 260, 80}, 1, Color{70,70,70,255});
        DrawText("Czas wykonania:", 35, y+15, 16, COL_TEXT_MUTED);
        DrawText(TextFormat("%lld us", executionTime), 35, y+40, 30, COL_ACCENT);

        // --- SEKCJA RĘCZNEGO DODAWANIA WYNIKÓW ---
        y += 90;
        
        // Przycisk dodawania (aktywny tylko gdy mamy wynik > 0)
        bool canAdd = (executionTime > 0);
        if (DrawStyledButton({20, y, 180, 40}, "Dodaj wynik (+)", canAdd, COL_SUCCESS)) {
            if (canAdd) {
                benchmarkResults.push_back({currentLibName, threadCount, executionTime});
                showAddMessage = true;
                addMessageCounter = 60; 
            }
        }

        // Przycisk czyszczenia
        if (DrawStyledButton({210, y, 70, 40}, "Reset", true, COL_DANGER)) {
            benchmarkResults.clear();
        }

        y += 50;

        // Komunikaty statusu dodawania
        if (showAddMessage) {
            DrawText("Dodano do listy!", 20, y, 16, COL_SUCCESS);
            addMessageCounter--;
            if (addMessageCounter <= 0) showAddMessage = false;
        } else {
            DrawText(TextFormat("W buforze: %d wynikow", benchmarkResults.size()), 20, y, 16, COL_TEXT_MUTED);
        }

        // Przycisk ZAPISU DO CSV
        y += 30;
        if (DrawStyledButton({20, y, 260, 40}, "Zapisz CSV na dysk")) {
            ofstream file("benchmark_results.csv");
            file << "Library,Threads,Time_us\n";
            for(const auto& res : benchmarkResults) {
                file << res.libName << "," << res.threads << "," << res.timeUs << "\n";
            }
            file.close();
            showSaveMessage = true;
            saveMessageCounter = 120;
        }

        if (showSaveMessage) {
            DrawText("Zapisano plik CSV!", 70, y + 50, 16, COL_SUCCESS);
            saveMessageCounter--;
            if (saveMessageCounter <= 0) showSaveMessage = false;
        }

        // Instrukcja na dole
        y = screenHeight - 150;
        DrawText("INSTRUKCJA:", 20, y, 16, COL_ACCENT);
        DrawText("- Przetworz obraz", 20, y+20, 14, COL_TEXT_MUTED);
        DrawText("- Kliknij 'Dodaj wynik (+)'", 20, y+40, 14, COL_TEXT_MUTED);
        DrawText("- Na koniec 'Zapisz CSV'", 20, y+60, 14, COL_TEXT_MUTED);

        // Obszar roboczy
        Rectangle workArea = { 300, 0, screenWidth - 300.0f, screenHeight - 160.0f };
        if (imageLoaded) {
            float margin = 20;
            float availableW = (workArea.width - 3*margin) / 2.0f;
            float availableH = workArea.height - 2*margin;
            float scale = min(availableW / originalTexture.width, availableH / originalTexture.height);
            float drawW = originalTexture.width * scale;
            float drawH = originalTexture.height * scale;
            
            float x1 = workArea.x + margin + (availableW - drawW)/2;
            float y1 = margin + (availableH - drawH)/2;
            DrawText("Oryginal", x1, y1 - 25, 20, COL_TEXT);
            DrawTexturePro(originalTexture, {0,0,(float)originalTexture.width,(float)originalTexture.height}, {x1, y1, drawW, drawH}, {0,0}, 0, WHITE);
            DrawRectangleLinesEx({x1, y1, drawW, drawH}, 2, Color{60,60,60,255});

            float x2 = workArea.x + 2*margin + availableW + (availableW - drawW)/2;
            DrawText(TextFormat("Wynik [%s]", currentLibName.c_str()), x2, y1 - 25, 20, COL_ACCENT);
            DrawTexturePro(resultTexture, {0,0,(float)resultTexture.width,(float)resultTexture.height}, {x2, y1, drawW, drawH}, {0,0}, 0, WHITE);
            DrawRectangleLinesEx({x2, y1, drawW, drawH}, 2, COL_ACCENT);

            // Histogram
            DrawRectangleRec(histogramRect, Color{25,25,30,255});
            DrawRectangleLinesEx(histogramRect, 1, Color{60,60,60,255});
            
            for(size_t i=0; i<thresholds.size(); i++) {
                float tx = histogramRect.x + ((float)thresholds[i]/255.0f) * histogramRect.width;
                Color thColor = ((int)i == hoveredThresholdIdx || (int)i == draggingThresholdIdx) ? COL_HIGHLIGHT : COL_DANGER;
                DrawLine(tx, histogramRect.y, tx, histogramRect.y + histogramRect.height, thColor);
                DrawTriangle({tx-6, histogramRect.y}, {tx+6, histogramRect.y}, {tx, histogramRect.y+8}, thColor);
                DrawTriangle({tx-6, histogramRect.y+histogramRect.height}, {tx, histogramRect.y+histogramRect.height-8}, {tx+6, histogramRect.y+histogramRect.height}, thColor);
                if((int)i == hoveredThresholdIdx || (int)i == draggingThresholdIdx) {
                     DrawText(TextFormat("%d", thresholds[i]), tx+8, histogramRect.y + 10, 20, COL_TEXT);
                }
            }
            DrawText("Edytor Progow", histogramRect.x + 10, histogramRect.y + 10, 16, Color{100,100,100,255});

        } else {
            DrawText("Upusc tutaj plik obrazu (.png)", workArea.x + workArea.width/2 - 150, workArea.height/2, 24, COL_TEXT_MUTED);
        }

        EndDrawing();
    }

    if (imageLoaded) { UnloadTexture(originalTexture); UnloadTexture(resultTexture); UnloadImage(originalImage); UnloadImage(resultImage); }
    if (libHandle) dlclose(libHandle);
    CloseWindow();
    return 0;
}