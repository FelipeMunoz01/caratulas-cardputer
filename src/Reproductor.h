// Reproduce un GIF de la microSD sobre el lienzo de 240 x 135.
//
// Centra la imagen y, si no cabe, la reduce por un divisor entero (mitad,
// tercio...). No agranda: un GIF chico se ve chico y centrado, que es mejor
// que verlo borroso.
#pragma once

#include <M5Cardputer.h>

class Reproductor {
public:
    void inicializar(M5Canvas* lienzo);

    bool abrir(const char* ruta);
    void cerrar();
    bool abierto() const { return abierto_; }

    // Dibuja el siguiente cuadro. Devuelve cuántos ms hay que esperar antes
    // del próximo, o -1 si la animación terminó y hay que volver a abrirla.
    int cuadro();

    int anchoOriginal() const { return ancho_; }
    int altoOriginal() const { return alto_; }
    int divisor() const { return divisor_; }

private:
    bool abierto_ = false;
    int  ancho_ = 0;
    int  alto_ = 0;
    int  divisor_ = 1;
};
