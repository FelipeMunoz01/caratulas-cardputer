// La microSD: montarla y leer qué carátulas hay.
#pragma once

#include "Lista.h"

namespace tarjeta {

// En el ADV no basta con SD.begin(): hay chips I2C nuevos que interfieren con
// el chip select del SPI y la tarjeta no monta. Esto aplica el remedio.
bool montar();

bool montada();

// Llena la lista con los .gif de esa carpeta. Devuelve cuántos encontró.
int listar(caratulas::Lista& lista, const char* carpeta);

}  // namespace tarjeta
