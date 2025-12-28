// Plik: image_processing_asm.s
// Funkcja asemblerowa dla architektury ARM64 z użyciem instrukcji NEON
// do jednoprogowego przetwarzania obrazu.

.global _ImageSingleThreshold_NEON // Upublicznia symbol dla linkera (prefix '_' dla macOS ABI)
.align 4                           // Wyrównanie kodu do 4 bajtów

// void ImageSingleThreshold_NEON(unsigned int* pixels, long pixelCount, unsigned char threshold, unsigned int colorBelow, unsigned int colorAbove);
// Argumenty (zgodnie z ARM64 ABI):
// x0: unsigned int* pixels
// x1: long pixelCount
// w2: unsigned char threshold
// w3: unsigned int colorBelow
// w4: unsigned int colorAbove

_ImageSingleThreshold_NEON:
    // --- Prolog funkcji ---
    // Zapisujemy rejestry, które funkcja modyfikuje, a które powinny być zachowane (callee-saved)
    stp x19, x20, [sp, #-32]! // Zapisz x19, x20 na stosie i przesuń wskaźnik stosu
    stp x21, x22, [sp, #16]   // Zapisz x21, x22

    // --- Przygotowanie danych ---
    mov x19, x0               // x19 = wskaźnik na piksele
    mov x20, x1               // x20 = liczba pikseli

    // Przygotowanie wektorów NEON z kolorami i progiem
    dup v5.16b, w2            // Wypełnij wektor v5 wartością progu (16 bajtów)
    dup v6.4s, w3             // Wypełnij wektor v6 kolorem 'poniżej' (4x 32-bit int)
    dup v7.4s, w4             // Wypełnij wektor v7 kolorem 'powyżej' (4x 32-bit int)

    // Obliczenie liczby pikseli do przetworzenia w pętli (wielokrotność 16)
    and x21, x20, #~15        // limit = pixelCount - (pixelCount % 16)
    cmp x21, #0
    beq .Lprocess_remainder   // Jeśli nie ma pełnych bloków 16-pikselowych, przejdź do reszty

    mov x22, x19              // x22 = wskaźnik wyjściowy (będziemy go inkrementować)

    // --- Główna pętla przetwarzania (16 pikseli na iterację) ---
.Lloop_start:
    // Wczytaj 4x4 piksele (16 pikseli = 64 bajty), rozdzielając kanały R,G,B,A
    ld4 {v0.16b, v1.16b, v2.16b, v3.16b}, [x19], #64 // Wczytaj i przesuń wskaźnik wejściowy
    // v0 zawiera teraz 16 składowych R z 16 kolejnych pikseli

    // Porównaj składowe R z progiem (unsigned compare higher)
    // Wynikowa maska zostanie zapisana w rejestrze v4
    cmhi v4.16b, v0.16b, v5.16b

    // Wybierz bitowo (Bitwise Select) między kolorem 'above' a 'below' na podstawie maski z v4
    // Wynikowe 16 pikseli (64 bajty) zostanie zapisane w rejestrach v0, v1, v2, v3
    bsl v4.16b, v7.16b, v6.16b // v4 teraz zawiera wynikowe piksele
    mov v0.16b, v4.16b         // Kopiujemy wynik do pozostałych rejestrów,
    mov v1.16b, v4.16b         // ponieważ każdy piksel w bloku 16 ma być taki sam
    mov v2.16b, v4.16b
    mov v3.16b, v4.16b

    // Zapisz wynikowe 16 pikseli (64 bajty) do pamięci, przeplatając dane z v0, v1, v2, v3
    st4 {v0.16b, v1.16b, v2.16b, v3.16b}, [x22], #64

    subs x21, x21, #16        // Zmniejsz licznik pętli
    bne .Lloop_start          // Jeśli licznik > 0, kontynuuj pętlę

.Lprocess_remainder:
    // --- Epilog funkcji ---
    ldp x21, x22, [sp, #16]   // Odtwórz x21, x22
    ldp x19, x20, [sp], #32   // Odtwórz x19, x20 i przywróć wskaźnik stosu
    ret                       // Powrót z funkcji