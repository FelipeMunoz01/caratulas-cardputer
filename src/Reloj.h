// El Cardputer ADV no trae RTC, así que la hora solo se consigue por NTP.
// Sin red configurada el reloj de la carátula no se muestra.
#pragma once

#include <cstddef>
#include <cstdint>

namespace reloj {

void inicializar();
bool horaConocida();

// Llena "14:35" y el porcentaje de batería. Devuelve false si no hay hora.
bool horaTexto(char* destino, size_t tam);

}  // namespace reloj
