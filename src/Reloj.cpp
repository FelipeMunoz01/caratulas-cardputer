#include "Reloj.h"

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#if __has_include("secretos.h")
#include "secretos.h"
#define CARATULAS_TIENE_SECRETOS 1
#endif

namespace reloj {
namespace {
bool sincronizado = false;
constexpr uint32_t kEsperaWifiMs = 8000;
constexpr uint32_t kEsperaNtpMs  = 6000;
constexpr int64_t  kEpochMinimo  = 1700000000;
}  // namespace

void inicializar() {
#ifdef CARATULAS_TIENE_SECRETOS
    WiFi.mode(WIFI_STA);
    WiFi.begin(kWifiRed, kWifiClave);
    const uint32_t limite = millis() + kEsperaWifiMs;
    while (WiFi.status() != WL_CONNECTED && millis() < limite) delay(100);
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_OFF);
        Serial.println("[reloj] sin wifi: la caratula va sin hora");
        return;
    }

    // Zona horaria en formato POSIX. kZonaHoraria sale de secretos.h.
    configTzTime(kZonaHoraria, "pool.ntp.org", "time.cloudflare.com");
    const uint32_t limiteNtp = millis() + kEsperaNtpMs;
    while (time(nullptr) < kEpochMinimo && millis() < limiteNtp) delay(100);
    sincronizado = time(nullptr) >= kEpochMinimo;

    // La radio se apaga: la hora ya quedo en el reloj interno del chip y asi
    // la bateria dura mucho mas.
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.printf("[reloj] %s\n", sincronizado ? "en hora" : "NTP no respondio");
#else
    Serial.println("[reloj] sin secretos.h: la caratula va sin hora");
#endif
}

bool horaConocida() { return sincronizado; }

bool horaTexto(char* destino, size_t tam) {
    if (!sincronizado) return false;
    const time_t ahora = time(nullptr);
    struct tm local;
    localtime_r(&ahora, &local);
    strftime(destino, tam, "%H:%M", &local);
    return true;
}

}  // namespace reloj
