cmake_minimum_required(VERSION 3.10)
project(JA_Projekt LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
    set(OSX_FRAMEWORKS 
        "-framework CoreVideo" 
        "-framework IOKit" 
        "-framework Cocoa" 
        "-framework GLUT" 
        "-framework OpenGL"
        "-framework CoreFoundation"
    )
endif()

#Biblioteka C++
add_library(lib_cpp SHARED src/lib_cpp.cpp)
set_target_properties(lib_cpp PROPERTIES OUTPUT_NAME "algo_cpp")
target_include_directories(lib_cpp PRIVATE src)


#Wyłączamy optymalizację (-O0) tylko dla tej biblioteki
target_compile_options(lib_cpp PRIVATE -O0)

target_link_libraries(lib_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a ${OSX_FRAMEWORKS})

#Biblioteka ASM
add_library(lib_asm SHARED src/lib_asm_wrapper.cpp src/image_processing_asm.s)
set_target_properties(lib_asm PROPERTIES OUTPUT_NAME "algo_asm")
target_include_directories(lib_asm PRIVATE src)
target_link_libraries(lib_asm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a ${OSX_FRAMEWORKS})

#Aplikacja glowna
set(TARGET_NAME app)
add_executable(${TARGET_NAME} src/main.cpp src/histogram.cpp)
target_include_directories(${TARGET_NAME} PRIVATE src)
target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a dl ${OSX_FRAMEWORKS})