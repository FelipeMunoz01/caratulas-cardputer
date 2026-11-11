// Copia este archivo como src/secretos.h y pon los datos de tu red.
// secretos.h está en .gitignore, no se sube al repo.
//
// Sirve solo para poner el reloj en hora por NTP al prender. Sin esto las
// carátulas se ven igual, pero sin la hora encima.
#pragma once

constexpr const char* kWifiRed   = "nombre de tu red";
constexpr const char* kWifiClave = "clave de tu red";

// Zona horaria en formato POSIX. Esta es la de Chile continental, con el
// cambio de hora incluido. Para otras: https://github.com/nayarsystems/posix_tz_db
constexpr const char* kZonaHoraria = "<-04>4<-03>,M9.1.6/24,M4.1.6/24";
