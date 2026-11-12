#include "Capa.h"

#include <cstdio>
#include <cstring>

#include "Reloj.h"

namespace capa {
namespace {

// Texto con sombra, para que se lea sobre cualquier imagen.
void conSombra(int x, int y, const char* texto, uint32_t color) {
    auto& p = M5Cardputer.Display;
    p.setTextColor(p.color565(0, 0, 0));
    p.drawString(texto, x + 2, y + 2);
    p.setTextColor(color);
    p.drawString(texto, x, y);
}

}  // namespace

void dibujar(bool mostrarReloj) {
    auto& p = M5Cardputer.Display;

    if (!mostrarReloj) return;

    p.setFont(&fonts::Font2);
    p.setTextSize(1);

    // Bateria arriba a la derecha.
    char bat[8];
    snprintf(bat, sizeof(bat), "%d%%", M5Cardputer.Power.getBatteryLevel());
    conSombra(p.width() - p.textWidth(bat) - 8, 6, bat, p.color565(230, 235, 240));

    // Hora abajo a la derecha, del mismo porte que la bateria.
    char hora[8];
    if (reloj::horaTexto(hora, sizeof(hora))) {
        conSombra(p.width() - p.textWidth(hora) - 8, p.height() - 24, hora,
                  p.color565(255, 255, 255));
    }
}

}  // namespace capa
