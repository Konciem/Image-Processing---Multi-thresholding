# Multi-Thresholding - Projekt Języki Asemblerowe

## Opis projektu
Projekt realizuje aplikację do **wieloprogowania obrazu** w czasie rzeczywistym. Program pozwala na segmentację obrazu poprzez nakładanie wielu progów jasności, co umożliwia wyodrębnienie konkretnych detali z grafiki i przypisanie im zdefiniowanych przez użytkownika kolorów.

Głównym założeniem projektu jest pokazanie różnic w podejściu do programowania oraz wydajności między językiem wysokiego poziomu (C++) a niskopoziomowym asemblerem.

## Cel projektu
Kluczowym celem jest **porównanie wydajności** dwóch implementacji tego samego algorytmu przetwarzania obrazu:
1.  **Implementacja w C++**: Wykorzystująca standardowe mechanizmy języka wysokiego poziomu.
2.  **Implementacja w Asemblerze (ARM64 NEON)**: Zoptymalizowana pod kątem architektury procesora (Apple M1), wykorzystująca instrukcje wektorowe do równoległego przetwarzania wielu pikseli jednocześnie.

## Wykorzystane technologie
* **Język główny**: C++
* **Asembler**: ARM64 (instrukcje NEON) – wykorzystywany do niskopoziomowej optymalizacji obliczeń.
* **Biblioteka graficzna**: **Raylib** – użyta do stworzenia okienkowego interfejsu użytkownika, obsługi tekstur oraz renderowania UI.
* **System budowania**: CMake.

## Główne funkcje
* **Przetwarzanie w czasie rzeczywistym**: Zmiany progów lub kolorów są natychmiast widoczne na podglądzie.
* **Interaktywny Histogram**: Wizualizacja rozkładu jasności pikseli, służąca jednocześnie jako panel sterowania progami.
* **System Drag & Drop**: Możliwość wczytania obrazu PNG poprzez przeciągnięcie go do okna aplikacji.
* **Edytor Palety**: Intuicyjne suwaki RGB pozwalające na dostosowanie kolorów dla każdego zakresu jasności.
* **Zarządzanie progami**: Dodawanie (SHIFT + PPM) i usuwanie (CTRL + PPM) progów bezpośrednio na wykresie histogramu.

<img width="1287" height="939" alt="Image" src="https://github.com/user-attachments/assets/2b56962f-3061-4c17-8483-3576d1e6e5bf" />

## Zasada działania
1.  **Analiza jasności**: Wczytany obraz jest konwertowany do skali szarości, gdzie każdy piksel przyjmuje wartość jasności.
2.  **Podział na przedziały**: Użytkownik definiuje progi, które tworzą przedziały jasności (np. 0-80, 81-160, itd.).
3.  **Mapowanie kolorów**: Algorytm sprawdza jasność każdego piksela i przypisuje mu kolor odpowiadający danemu przedziałowi.
4.  **Optymalizacja**: W wersji asemblerowej proces ten odbywa się grupowo – procesor wykorzystuje rejestry wektorowe do przetwarzania wielu pikseli w jednej instrukcji, co znacząco przyspiesza generowanie obrazu wynikowego.
