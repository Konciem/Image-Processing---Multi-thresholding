# Wymagana minimalna wersja CMake
cmake_minimum_required(VERSION 3.10)

# Definicja projektu. Kluczowe jest dodanie języka ASM, aby CMake wiedział,
# jak obsługiwać pliki .s (dla ARM będzie to clang, dla innych architektur GNU as).
project(JA_Projekt LANGUAGES CXX ASM)

# --- Konfiguracja kompilacji ---

# Ustawienie standardu C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodanie flag kompilatora (analogicznie do CXXFLAGS w Makefile)
add_compile_options(-Wall -Wextra)

# --- Definicja celu (aplikacji) ---

# Nazwa pliku wykonywalnego
set(TARGET_NAME app)

# Automatyczne wyszukiwanie plików źródłowych C++ i asemblera
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB ASM_SOURCES "*.s")
set(ASM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/image_processing_asm.s)

# Stworzenie pliku wykonywalnego z podanych źródeł
add_executable(${TARGET_NAME} ${CPP_SOURCES} ${ASM_SOURCES})

# Dodanie katalogu 'src' do ścieżek przeszukiwania plików nagłówkowych.
# Dzięki temu #include "image_processing.hpp" zadziała poprawnie.
target_include_directories(${TARGET_NAME} PRIVATE src)

# --- Konfiguracja linkowania ---

# Dodanie biblioteki Raylib (zakładamy, że libraylib.a jest w tym samym katalogu)
# target_link_libraries łączy bibliotekę z naszym celem (aplikacją)
target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a)

# Dodawanie frameworków systemowych dla macOS (analogicznie do LDFLAGS w Makefile)
if(APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE
        "-framework CoreVideo"
        "-framework IOKit"
        "-framework Cocoa"
        "-framework GLUT"
        "-framework OpenGL"
    )
endif()

# --- Dodatkowe reguły (opcjonalnie) ---

# Można dodać niestandardową regułę "run", podobną do tej z Makefile
add_custom_target(run COMMAND ./${TARGET_NAME} DEPENDS ${TARGET_NAME})