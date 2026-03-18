# Multi-Thresholding - Assembly Languages Project

## Project Description
The project implements a real-time **image multi-thresholding** application. The program allows for image segmentation by applying multiple brightness thresholds, which enables the extraction of specific details from the graphics and assigning them user-defined colors.

The main objective of the project is to demonstrate the differences in programming approach and performance between a high-level language (C++) and low-level Assembly.

## Project Goal
The key goal is to **compare the performance** of two implementations of the same image processing algorithm:
1. **C++ Implementation**: Utilizing standard high-level language mechanisms.
2. **Assembly Implementation (ARM64 NEON)**: Optimized for the processor architecture (Apple M1), using vector instructions for parallel processing of multiple pixels simultaneously.

## Technologies Used
* **Main Language**: C++
* **Assembly**: ARM64 (NEON instructions) – used for low-level calculation optimization.
* **Graphics Library**: **Raylib** – used to create the windowed user interface, handle textures, and render the UI.
* **Build System**: CMake.

## Main Features
* **Real-time Processing**: Changes to thresholds or colors are immediately visible in the preview.
* **Interactive Histogram**: Visualization of pixel brightness distribution, serving simultaneously as a threshold control panel.
* **Drag & Drop System**: Ability to load PNG images by dragging them into the application window.
* **Palette Editor**: Intuitive RGB sliders allowing for color adjustment for each brightness range.
* **Threshold Management**: Adding (SHIFT + RMB) and removing (CTRL + RMB) thresholds directly on the histogram chart.

<img width="1287" height="939" alt="Image" src="https://github.com/user-attachments/assets/2b56962f-3061-4c17-8483-3576d1e6e5bf" />

## Principle of Operation
1. **Brightness Analysis**: The loaded image is converted to grayscale, where each pixel takes on a brightness value.
2. **Division into Intervals**: The user defines thresholds that create brightness intervals (e.g., 0-80, 81-160, etc.).
3. **Color Mapping**: The algorithm checks the brightness of each pixel and assigns it a color corresponding to the given interval.
4. **Optimization**: In the assembly version, this process takes place in groups – the processor uses vector registers to process multiple pixels in a single instruction, which significantly speeds up the generation of the resulting image.
