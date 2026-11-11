// El reloj y la batería encima de la carátula.
//
// Se dibuja directo sobre la pantalla, no sobre el lienzo, a propósito: el
// lienzo tiene que quedar con el GIF puro, porque los cuadros siguientes se
// dibujan encima de los anteriores y la hora se iría quedando pegada.
#pragma once

#include <M5Cardputer.h>

namespace capa {

void dibujar(bool mostrarReloj);

// Nombre de la carátula, abajo a la derecha, por unos segundos al cambiarla.
void anunciar(const char* nombre);

}  // namespace capa
