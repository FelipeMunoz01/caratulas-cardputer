// Carátulas: fondos animados para el M5Stack Cardputer ADV.
//
// Lee los .gif de /caratulas en la microSD y los reproduce a pantalla
// completa, con la hora y la batería encima.
//
// Teclas:  , anterior    / siguiente    O reloj si o no
//          - y =  brillo    R  releer la tarjeta
#include <Arduino.h>
#include <M5Cardputer.h>
#include <Preferences.h>

#include "Capa.h"
#include "Lista.h"
#include "Reloj.h"
#include "Reproductor.h"
#include "Tarjeta.h"

namespace {

constexpr const char* kCarpeta = "/caratulas";
constexpr const char* kEspacioNVS = "caratulas";

M5Canvas          lienzo(&M5Cardputer.Display);
caratulas::Lista  lista;
Reproductor       reproductor;

bool     mostrarReloj = true;
uint8_t  brillo = 110;
uint32_t proximoCuadroMs = 0;

void guardarAjustes() {
    Preferences p;
    if (!p.begin(kEspacioNVS, false)) return;
    p.putString("ultima", lista.actual().c_str());
    p.putBool("reloj", mostrarReloj);
    p.putUChar("brillo", brillo);
    p.end();
}

void cargarAjustes() {
    Preferences p;
    if (!p.begin(kEspacioNVS, true)) return;
    mostrarReloj = p.getBool("reloj", true);
    brillo = p.getUChar("brillo", 110);
    const String ultima = p.getString("ultima", "");
    p.end();
    if (ultima.length() > 0) lista.recuperar(ultima.c_str());
}

// Abre la carátula actual y deja todo listo para el primer cuadro.
void abrirActual() {
    reproductor.cerrar();
    lienzo.fillSprite(TFT_BLACK);
    if (lista.vacia()) return;

    char ruta[160];
    snprintf(ruta, sizeof(ruta), "%s/%s", kCarpeta, lista.actual().c_str());
    if (reproductor.abrir(ruta)) {
        capa::anunciar(lista.actual().c_str());
    }
    proximoCuadroMs = millis();
}

void cambiar(bool haciaAdelante) {
    if (lista.cantidad() < 2) return;
    haciaAdelante ? lista.siguiente() : lista.anterior();
    abrirActual();
    guardarAjustes();
}

void releerTarjeta() {
    const std::string antes = lista.actual();
    lista = caratulas::Lista{};
    tarjeta::listar(lista, kCarpeta);
    if (!antes.empty()) lista.recuperar(antes);
    abrirActual();
}

// Vuelve al Launcher. El Launcher arranca primero y le cede el control a la
// aplicacion instalada si no se aprieta nada, asi que reiniciar es exactamente
// lo que hace el boton RST, pero desde el teclado.
void volverAlLauncher() {
    guardarAjustes();

    auto& p = M5Cardputer.Display;
    p.fillScreen(p.color565(14, 16, 22));
    p.setFont(&fonts::Font2);
    p.setTextColor(p.color565(235, 238, 242));
    p.drawString("Volviendo al Launcher...", 10, 50);
    p.setTextColor(p.color565(150, 160, 175));
    p.drawString("manten una tecla apretada", 10, 76);
    delay(900);

    ESP.restart();
}

// Mensaje en pantalla cuando no hay nada que mostrar, para que no quede negra
// sin explicacion.
void pantallaDeAyuda(const char* motivo) {
    auto& p = M5Cardputer.Display;
    p.fillScreen(p.color565(14, 16, 22));
    p.setFont(&fonts::Font2);
    p.setTextColor(p.color565(235, 238, 242));
    p.drawString("Caratulas", 10, 12);
    p.setTextColor(p.color565(150, 160, 175));
    p.drawString(motivo, 10, 42);
    p.drawString("Pon archivos .gif en", 10, 66);
    p.drawString("/caratulas de la microSD", 10, 86);
    p.drawString("y aprieta R", 10, 106);
}

void atenderTeclado() {
    if (!M5Cardputer.Keyboard.isChange()) return;
    if (!M5Cardputer.Keyboard.isPressed()) return;

    const auto estado = M5Cardputer.Keyboard.keysState();

    // Backspace tambien sale, porque es la otra tecla que el Launcher usa como
    // "volver" y es la que la gente prueba.
    if (estado.del) {
        volverAlLauncher();
        return;
    }

    for (const char c : estado.word) {
        switch (c) {
            case ',': cambiar(false); return;
            case '/': cambiar(true);  return;
            case 'o':
                mostrarReloj = !mostrarReloj;
                M5Cardputer.Display.fillScreen(TFT_BLACK);
                guardarAjustes();
                return;
            case '-':
                brillo = (brillo > 20) ? brillo - 20 : 10;
                M5Cardputer.Display.setBrightness(brillo);
                guardarAjustes();
                return;
            case '=':
                brillo = (brillo < 235) ? brillo + 20 : 255;
                M5Cardputer.Display.setBrightness(brillo);
                guardarAjustes();
                return;
            case 'r': releerTarjeta(); return;
            // La tecla esc del Cardputer no llega como un codigo aparte: llega
            // como el caracter de acento grave.
            case '`': volverAlLauncher(); return;
            default: break;
        }
    }
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    Serial.begin(115200);

    auto& p = M5Cardputer.Display;
    // Nada de setRotation fijo: M5Cardputer.begin() ya deja la que corresponde
    // a la placa, y forzarla deja el ADV de costado.
    if (p.width() < p.height()) p.setRotation(p.getRotation() ^ 1);
    Serial.printf("[pantalla] %d x %d\n", p.width(), p.height());

    lienzo.setColorDepth(16);
    if (!lienzo.createSprite(p.width(), p.height())) {
        Serial.println("[pantalla] no alcanzo la memoria para el lienzo");
    }
    reproductor.inicializar(&lienzo);

    if (!tarjeta::montar()) {
        pantallaDeAyuda("No se pudo leer la microSD");
        return;
    }
    tarjeta::listar(lista, kCarpeta);

    cargarAjustes();
    p.setBrightness(brillo);
    reloj::inicializar();

    if (lista.vacia()) {
        pantallaDeAyuda("No hay ningun .gif todavia");
        return;
    }
    abrirActual();
}

void loop() {
    M5Cardputer.update();
    atenderTeclado();

    if (lista.vacia() || !tarjeta::montada()) {
        delay(50);
        return;
    }

    const uint32_t ahora = millis();
    if (ahora >= proximoCuadroMs) {
        const int espera = reproductor.cuadro();
        if (espera < 0) {
            abrirActual();          // terminó el bucle: vuelve a empezar
        } else {
            lienzo.pushSprite(0, 0);
            capa::dibujar(mostrarReloj);
            // Un GIF sin retardo declarado no debe comerse el procesador.
            proximoCuadroMs = ahora + (espera > 10 ? espera : 40);
        }
    }

    delay(2);
}
