// El reloj y la batería encima de la carátula.
//
// Los dos van chicos y en las esquinas, a propósito: lo que importa es la
// imagen, no los datos. El nombre del archivo no se muestra porque ensuciaba
// la carátula sin aportar nada.
//
// Se dibuja directo sobre la pantalla, no sobre el lienzo, a propósito: el
// lienzo tiene que quedar con el GIF puro, porque los cuadros siguientes se
// dibujan encima de los anteriores y la hora se iría quedando pegada.
#pragma once

#include <M5Cardputer.h>

namespace capa {

void dibujar(bool mostrarReloj);

}  // namespace capa
