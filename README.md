# Carátulas (Cardputer ADV)

Fondos animados para el M5Stack Cardputer ADV. Lee los `.gif` de la microSD y
los reproduce a pantalla completa, con la hora y la batería encima.

Un solo firmware, todas las carátulas que quieras: se cambian dejando archivos
en la tarjeta, sin recompilar ni volver a flashear.

## Cómo se usa

1. En la microSD, crea una carpeta `/caratulas`.
2. Deja ahí tus archivos `.gif`.
3. Enciende el Cardputer.

| Tecla | Qué hace |
|---|---|
| `,` | carátula anterior |
| `/` | carátula siguiente |
| `O` | mostrar u ocultar el reloj |
| `-` y `=` | bajar y subir el brillo |
| `R` | releer la tarjeta, por si agregaste archivos |

Lo que elijas se guarda: al prender vuelve a la última carátula, con el mismo
brillo y el reloj como lo dejaste.

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
~/.venvs/imagen/bin/python herramientas/ajustar.py entrada.gif salida.gif
```

Recorta al centro a 16:9 y reduce a 240 x 135. Con `--caber` encaja la imagen
entera dejando barras negras, para cuando recortar se comería algo importante.
Escribe cada cuadro completo en vez de solo las diferencias: el archivo queda
algo más grande, pero el equipo no depende del cuadro anterior para dibujar.

Necesita Pillow, que vive en `~/.venvs/imagen`.

La tarjeta tiene que ser **SDHC de 32 GB o menos, formateada en FAT32**. Las
SDXC no las lee.

## El reloj

El Cardputer ADV no tiene RTC, así que la hora llega por NTP al prender. Copia
`src/secretos.ejemplo.h` a `src/secretos.h` y pon tu red; ese archivo está en
`.gitignore`. Sin red configurada todo funciona igual, pero sin hora.

La radio se apaga apenas sincroniza, así que no gasta batería durante el resto
del rato.

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
~/.venvs/platformio/bin/pio run -e cardputer-adv
~/.venvs/platformio/bin/pio test -e pruebas
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
