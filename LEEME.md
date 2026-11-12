# Carátulas (Cardputer ADV)

*[English README](README.md)*

Fondos animados para el M5Stack Cardputer ADV. Lee los `.gif` de la microSD y
los reproduce a pantalla completa, con la hora y la batería encima.

Un solo firmware, todas las carátulas que quieras: se cambian dejando archivos
en la tarjeta, sin recompilar ni volver a flashear.

## Cómo se usa

1. En la microSD, crea una carpeta `/caratulas`.
2. Deja ahí tus archivos `.gif`.
3. Enciende el Cardputer.

Para agregar carátulas después **no hay que recompilar ni reflashear nada**:
copias más `.gif` a esa carpeta y aprietas `R`. Ni siquiera hace falta sacar la
tarjeta, porque la WebUI del Launcher deja subir archivos por WiFi.

## Instalarlo sin perder el Launcher

Instálalo **desde el menú del Launcher**, nunca por cable. El Launcher
reconstruye la tabla de particiones al instalar y se queda: al encender arranca
Carátulas, y si aprietas una tecla entras al Launcher.

Flashear por cable a 0x0 sí borra todo lo que haya, incluido el Launcher.

| Tecla | Qué hace |
|---|---|
| `,` | carátula anterior |
| `/` | carátula siguiente |
| `O` | mostrar u ocultar los datos de encima |
| `-` y `=` | bajar y subir el brillo |
| `R` | releer la tarjeta, por si agregaste archivos |
| `ESC` o Backspace | volver al Launcher |

Lo que elijas se guarda: al prender vuelve a la última carátula, con el mismo
brillo y el reloj como lo dejaste.

`ESC` reinicia el equipo, que es lo mismo que hace el botón RST. En la pantalla
de arranque del Launcher hay que apretar **ENTER**: cualquier otra tecla dispara
su fastboot y salta de vuelta acá. El menú CFG del Launcher además tiene una
opción para arrancar siempre en el Launcher.

La tecla `esc` del Cardputer no llega al programa como un código propio sino
como el caracter de acento grave. Por eso también responde Backspace, que es la
otra tecla que el Launcher usa para volver.

## Sobre los GIF

No hace falta que midan 240 x 135. El reproductor **centra la imagen y, si no
cabe, la reduce por un divisor entero**: a la mitad, a un tercio, y así. Un GIF
de 500 x 311 se ve a 167 x 104 centrado. No agranda nunca.

**Pero conviene convertirlos antes**, y no por el tamaño en pantalla sino por
el trabajo de decodificar. El firmware reduce al dibujar, pero el ESP32 igual
tiene que decodificar el cuadro completo primero. Un GIF de 640 x 360 son
230.000 píxeles por cuadro para mostrar 32.000: eso corre a uno o dos cuadros
por segundo. Para eso está la herramienta:

```bash
# un archivo
python3 herramientas/ajustar.py entrada.gif salida.gif

# o una carpeta entera, directo a la tarjeta
python3 herramientas/ajustar.py ~/gifs /Volumes/CARDPUTER/caratulas
```

Recorta al centro a 16:9 y reduce a 240 x 135. Con `--caber` encaja la imagen
entera dejando barras negras, para cuando recortar se comería algo importante.
Escribe cada cuadro completo en vez de solo las diferencias: el archivo queda
algo más grande, pero el equipo no depende del cuadro anterior para dibujar.

Necesita Pillow: `python3 -m pip install pillow`.

La tarjeta tiene que ser **SDHC de 32 GB o menos, formateada en FAT32**. Las
SDXC no las lee.

## El reloj (opcional, apagado por defecto)

De fábrica solo se muestra el nivel de batería. El Cardputer ADV no tiene RTC,
así que la hora tendría que venir por NTP, y **arrastrar toda la pila de WiFi
cuesta unos 400 KB**, casi el doble del programa completo. Por eso el reloj hay
que pedirlo.

Para activarlo, copia `src/secretos.ejemplo.h` a `src/secretos.h` y pon tu red;
ese archivo está en `.gitignore`. Sin él no se compila nada de WiFi.

La radio se apaga apenas sincroniza, así que solo gasta batería una vez al
arrancar.

## Instalarlo

Desde el **Launcher**, que es lo cómodo: en OTA puedes instalarlo desde un
enlace de GitHub, desde la microSD o por WiFi con su WebUI. Usa el archivo
`caratulas-cardputer-adv-app.bin` de los releases.

Por cable, si prefieres:

```bash
esptool.py --chip esp32s3 --port /dev/cu.usbmodemXXXX --no-stub \
  write_flash 0x0 caratulas-cardputer-adv-completo.bin
```

Ojo: por cable reemplaza lo que haya en la placa, incluido el Launcher.

El `--no-stub` no es opcional: el USB-JTAG del S3 se cae al cargar el stub de
esptool cuando ya hay firmware corriendo.

## Compilar

```bash
pio run -e cardputer-adv
pio test -e pruebas
```

## Qué hay en la carpeta

```
lib/nucleo/
  Lista.h / Lista.cpp        qué carátulas hay y cuál se ve, sin Arduino
src/
  main.cpp                   arranque, teclado y bucle
  Tarjeta.h / .cpp           montar la microSD y listar los .gif
  Reproductor.h / .cpp       el GIF sobre el lienzo, centrado y reducido
  Capa.h / .cpp              el reloj y la batería encima
  Reloj.h / .cpp             hora por NTP
  secretos.ejemplo.h         plantilla para los datos de tu red
test/test_nucleo/
  test_lista.cpp             pruebas de la lista, corren en el Mac
herramientas/
  ajustar.py                 cualquier GIF a 240 x 135, para que corra fluido
```

## Dos cosas que costaron encontrarse

**La microSD no monta en el ADV con un `SD.begin()` normal.** Los chips I2C que
trae el ADV ensucian el chip select del SPI. Hay que dejar los GPIO 3, 4, 5, 6,
13 y 15 en alto antes de hablarle al bus. Sale de las notas del Launcher, que lo
probó en esta misma placa. Los pines son CS=12, SCK=40, MISO=39, MOSI=14.

**El reloj se dibuja sobre la pantalla, no sobre el lienzo.** Los cuadros de un
GIF se dibujan encima de los anteriores, así que si la hora se pintara en el
lienzo se iría quedando pegada, cuadro sobre cuadro.

## Qué está probado

En el Mac:

- Compila sin errores: 49 KB de RAM estática, 596 KB de flash.
- Las 8 pruebas de `test_lista.cpp` pasan: reconoce `.gif` sin importar
  mayúsculas, descarta la basura que deja macOS en las tarjetas (`._algo`),
  da la vuelta al pasar de una a otra, aguanta la lista vacía sin caerse,
  ordena alfabéticamente y recupera la última vista aunque ya no exista.

Todavía no verificado, hace falta el equipo con una tarjeta puesta:

- Que la microSD monte de verdad con el remedio de los GPIO.
- **La reproducción del GIF entera**, que es la parte con más riesgo: el
  centrado, la reducción por divisor entero y el manejo de transparencia entre
  cuadros.
- Que el lienzo de 65 KB y los buffers de AnimatedGIF convivan sin quedarse
  sin memoria.
