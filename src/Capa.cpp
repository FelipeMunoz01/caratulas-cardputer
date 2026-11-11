#include "Capa.h"

#include <cstdio>
#include <cstring>

#include "Reloj.h"

namespace capa {
namespace {

constexpr uint32_t kAnuncioMs = 2500;

char     anuncio[40] = {0};
uint32_t anuncioHastaMs = 0;

// Texto con sombra, para que se lea sobre cualquier imagen.
void conSombra(int x, int y, const char* texto, uint32_t color) {
    auto& p = M5Cardputer.Display;
    p.setTextColor(p.color565(0, 0, 0));
    p.drawString(texto, x + 2, y + 2);
    p.setTextColor(color);
    p.drawString(texto, x, y);
}

}  // namespace

void anunciar(const char* nombre) {
    snprintf(anuncio, sizeof(anuncio), "%s", nombre);
    anuncioHastaMs = millis() + kAnuncioMs;
}

void dibujar(bool mostrarReloj) {
    auto& p = M5Cardputer.Display;

    if (mostrarReloj) {
        char hora[8];
        if (reloj::horaTexto(hora, sizeof(hora))) {
            p.setFont(&fonts::Font7);
            p.setTextSize(1);
            conSombra(8, p.height() - 58, hora, p.color565(255, 255, 255));
        }

        p.setFont(&fonts::Font2);
        p.setTextSize(1);
        char bat[8];
        snprintf(bat, sizeof(bat), "%d%%", M5Cardputer.Power.getBatteryLevel());
        const int ancho = p.textWidth(bat);
        conSombra(p.width() - ancho - 8, 6, bat, p.color565(230, 235, 240));
    }

    if (anuncio[0] != '\0') {
        if (millis() < anuncioHastaMs) {
            p.setFont(&fonts::Font2);
            p.setTextSize(1);
            const int ancho = p.textWidth(anuncio);
            conSombra(p.width() - ancho - 8, p.height() - 24, anuncio,
                      p.color565(230, 235, 240));
        } else {
            anuncio[0] = '\0';
        }
    }
}

}  // namespace capa
