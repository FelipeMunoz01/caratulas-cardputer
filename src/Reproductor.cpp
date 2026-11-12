#include "Reproductor.h"

#include <AnimatedGIF.h>
#include <SD.h>

namespace {

AnimatedGIF gif;
File        archivo;

// El callback de dibujo es un puntero a función de C, así que lo que necesita
// saber vive acá al lado en vez de dentro de la clase.
M5Canvas* lienzo = nullptr;
int       desplazamientoX = 0;
int       desplazamientoY = 0;
int       divisorActivo = 1;

uint16_t linea[240];
bool     opaco[240];

void* abrirArchivo(const char* nombre, int32_t* tam) {
    archivo = SD.open(nombre, FILE_READ);
    if (!archivo) return nullptr;
    *tam = archivo.size();
    return &archivo;
}

void cerrarArchivo(void*) {
    if (archivo) archivo.close();
}

int32_t leerArchivo(GIFFILE* f, uint8_t* destino, int32_t largo) {
    File* a = static_cast<File*>(f->fHandle);
    const int32_t queda = f->iSize - f->iPos;
    if (largo > queda) largo = queda;
    if (largo <= 0) return 0;
    const int32_t leidos = a->read(destino, largo);
    f->iPos = a->position();
    return leidos;
}

int32_t buscarArchivo(GIFFILE* f, int32_t posicion) {
    File* a = static_cast<File*>(f->fHandle);
    a->seek(posicion);
    f->iPos = a->position();
    return f->iPos;
}

// Vuelca una línea del GIF en el lienzo, reduciendo y centrando.
void dibujarLinea(GIFDRAW* pDraw) {
    if (lienzo == nullptr) return;

    const int yOrigen = pDraw->iY + pDraw->y;
    if (yOrigen % divisorActivo != 0) return;  // fila que se salta al reducir

    const int yDestino = desplazamientoY + yOrigen / divisorActivo;
    if (yDestino < 0 || yDestino >= lienzo->height()) return;

    uint8_t*  origen = pDraw->pPixels;
    uint16_t* paleta = pDraw->pPalette;

    // Metodo 2 = restaurar el fondo: lo transparente pasa a ser el color de
    // fondo en vez de dejar ver el cuadro anterior.
    //
    // Hay que comprobar que el cuadro TENGA transparencia. Sin ella el indice
    // transparente vale cero, que es un color normal y muy usado, y se
    // terminaban pintando del color de fondo pixeles legitimos: puntitos
    // blancos sueltos, porque estos GIF declaran un fondo casi blanco.
    if (pDraw->ucDisposalMethod == 2 && pDraw->ucHasTransparency) {
        for (int x = 0; x < pDraw->iWidth; ++x) {
            if (origen[x] == pDraw->ucTransparent) origen[x] = pDraw->ucBackground;
        }
        pDraw->ucHasTransparency = 0;
    }

    const int anchoDestino = (pDraw->iWidth + divisorActivo - 1) / divisorActivo;
    int n = 0;
    for (int x = 0; x < pDraw->iWidth && n < 240; x += divisorActivo, ++n) {
        const uint8_t indice = origen[x];
        opaco[n] = !(pDraw->ucHasTransparency && indice == pDraw->ucTransparent);
        linea[n] = paleta[indice];
    }

    const int xBase = desplazamientoX + pDraw->iX / divisorActivo;

    if (!pDraw->ucHasTransparency) {
        lienzo->pushImage(xBase, yDestino, anchoDestino, 1, linea);
        return;
    }

    // Con transparencia se dibujan solo los tramos opacos, para que se vea lo
    // que haya debajo.
    int inicio = 0;
    while (inicio < n) {
        while (inicio < n && !opaco[inicio]) ++inicio;
        int fin = inicio;
        while (fin < n && opaco[fin]) ++fin;
        if (fin > inicio) {
            lienzo->pushImage(xBase + inicio, yDestino, fin - inicio, 1, &linea[inicio]);
        }
        inicio = fin;
    }
}

}  // namespace

void Reproductor::inicializar(M5Canvas* destino) {
    lienzo = destino;
    // BE y no LE: M5GFX interpreta los uint16_t de pushImage como RGB565 con
    // los bytes al reves, que es el formato en que viven las imagenes. Con LE
    // el dibujo sale bien pero los canales se cruzan, y el amarillo de Pikachu
    // se ve magenta. Comprobado en el equipo.
    gif.begin(GIF_PALETTE_RGB565_BE);
}

bool Reproductor::abrir(const char* ruta) {
    cerrar();
    if (!gif.open(ruta, abrirArchivo, cerrarArchivo, leerArchivo, buscarArchivo,
                  dibujarLinea)) {
        Serial.printf("[gif] no se pudo abrir %s (error %d)\n", ruta, gif.getLastError());
        return false;
    }

    ancho_ = gif.getCanvasWidth();
    alto_  = gif.getCanvasHeight();

    // Divisor entero mas chico con el que la imagen entra en la pantalla.
    divisor_ = 1;
    while ((ancho_ + divisor_ - 1) / divisor_ > lienzo->width() ||
           (alto_ + divisor_ - 1) / divisor_ > lienzo->height()) {
        ++divisor_;
        if (divisor_ > 16) break;  // red de seguridad, no deberia pasar
    }
    divisorActivo = divisor_;

    desplazamientoX = (lienzo->width() - (ancho_ + divisor_ - 1) / divisor_) / 2;
    desplazamientoY = (lienzo->height() - (alto_ + divisor_ - 1) / divisor_) / 2;
    if (desplazamientoX < 0) desplazamientoX = 0;
    if (desplazamientoY < 0) desplazamientoY = 0;

    abierto_ = true;
    Serial.printf("[gif] %s: %dx%d, divisor %d\n", ruta, ancho_, alto_, divisor_);
    return true;
}

void Reproductor::cerrar() {
    if (abierto_) {
        gif.close();
        abierto_ = false;
    }
}

int Reproductor::cuadro() {
    if (!abierto_) return -1;
    int esperaMs = 0;
    // El primer parametro en false: la espera la maneja el bucle principal,
    // que ademas tiene que atender el teclado.
    if (gif.playFrame(false, &esperaMs) == 0) return -1;  // se acabo el bucle
    return esperaMs;
}
