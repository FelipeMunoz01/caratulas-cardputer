#include "Tarjeta.h"

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

namespace tarjeta {
namespace {

// Pines verificados contra la configuración del Launcher, que está probada en
// el Cardputer ADV.
constexpr int kCS   = 12;
constexpr int kSCK  = 40;
constexpr int kMISO = 39;
constexpr int kMOSI = 14;

constexpr uint32_t kVelocidadSPI = 25000000;

bool lista_ = false;

}  // namespace

bool montar() {
    // El remedio del ADV: dejar estos pines en alto antes de hablarle al SPI.
    // Sin esto la tarjeta no monta, porque los chips I2C que trae el ADV
    // ensucian el chip select. Sacado de las notas del Launcher, que lo probó
    // en esta misma placa.
    for (int pin : {3, 4, 5, 6, 13, 15}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }

    SPI.begin(kSCK, kMISO, kMOSI, kCS);
    lista_ = SD.begin(kCS, SPI, kVelocidadSPI);
    if (!lista_) {
        // A veces engancha a la segunda, con el bus ya despierto.
        delay(50);
        lista_ = SD.begin(kCS, SPI, kVelocidadSPI);
    }
    Serial.printf("[tarjeta] %s\n", lista_ ? "montada" : "no monta");
    return lista_;
}

bool montada() { return lista_; }

int listar(caratulas::Lista& lista, const char* carpeta) {
    if (!lista_) return 0;

    File dir = SD.open(carpeta);
    if (!dir || !dir.isDirectory()) {
        Serial.printf("[tarjeta] no existe la carpeta %s\n", carpeta);
        if (dir) dir.close();
        return 0;
    }

    int encontradas = 0;
    for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
        if (!f.isDirectory()) {
            // name() puede venir con ruta completa segun la version del core.
            std::string nombre = f.name();
            const size_t barra = nombre.find_last_of('/');
            if (barra != std::string::npos) nombre = nombre.substr(barra + 1);
            if (lista.agregar(nombre)) ++encontradas;
        }
        f.close();
    }
    dir.close();
    lista.ordenar();

    Serial.printf("[tarjeta] %d caratulas en %s\n", encontradas, carpeta);
    return encontradas;
}

}  // namespace tarjeta
