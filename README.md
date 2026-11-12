# Caratulas: animated GIF wallpapers for the M5Stack Cardputer ADV

*[Versión en español](LEEME.md)*

Plays animated GIFs from the microSD card full screen, with a clock and battery
indicator on top. One firmware, as many wallpapers as you like: drop `.gif`
files on the card, no rebuilding and no reflashing.

**Built and tested on the Cardputer ADV**, which most firmware does not support
yet. See [Hardware support](#hardware-support) below.

## Install

The easy way, which keeps your Launcher: grab `caratulas-app.bin` from
[Releases](../../releases) and install it from
[M5Launcher](https://github.com/bmorcelli/M5Stick-Launcher) via **OTA** (from a
GitHub link, from the SD card, or over Wi-Fi with its WebUI).

Over USB instead, which **replaces whatever is on the board, Launcher
included**:

```bash
esptool.py --chip esp32s3 --port /dev/ttyACM0 --no-stub \
  write_flash 0x0 caratulas-complete.bin
```

`--no-stub` is not optional. The S3's USB-JTAG drops while esptool uploads its
stub once your own firmware is running: it connects, identifies the chip, then
dies with "No serial data received".

## Use

1. Create a `/caratulas` folder on the microSD.
2. Drop `.gif` files in it.
3. Power on.

| Key | Action |
|---|---|
| `,` | previous wallpaper |
| `/` | next wallpaper |
| `O` | show or hide the clock |
| `-` and `=` | brightness down and up |
| `R` | re-read the card, after adding files |

Your choice is remembered: it comes back to the last wallpaper, same
brightness, clock as you left it.

The card must be **SDHC, 32 GB or smaller, formatted FAT32**. SDXC will not
mount.

## Preparing GIFs

Any size works. The player centers the image and, when it does not fit, shrinks
it by a whole-number divisor. It never scales up.

**But converting first makes a big difference**, and not because of the size on
screen. The player shrinks while drawing, but the ESP32 still has to decode the
full frame first. A 640x360 GIF is 230,000 pixels decoded per frame to show
32,000 of them: that runs at one or two frames per second. Converted, it is
smooth.

```bash
python3 -m pip install pillow

# one file
python3 herramientas/ajustar.py input.gif output.gif

# or a whole folder, straight onto the card
python3 herramientas/ajustar.py ~/gifs /Volumes/CARDPUTER/caratulas
```

It center-crops to 16:9 and resizes to 240x135. Pass `--caber` to fit the whole
image with black bars instead, which is what you want when cropping would eat
something that matters.

## Hardware support

Tested on the **Cardputer ADV**. It should work on the original Cardputer too,
since the display, the SD pins and the M5Cardputer keyboard library are shared,
**but that is untested**. Reports welcome.

## Two things that cost time to find

Both are documented here because they are not obvious and may save someone else
the trouble.

**The microSD does not mount on the ADV with a plain `SD.begin()`.** The extra
I2C chips on the ADV interfere with the SPI chip select. GPIO 3, 4, 5, 6, 13
and 15 have to be driven HIGH before touching the bus. Credit to the M5Launcher
notes, which worked this out on the same board. Pins are CS=12, SCK=40,
MISO=39, MOSI=14.

**AnimatedGIF has to be initialised with `GIF_PALETTE_RGB565_BE`, not `_LE`.**
M5GFX reads the `uint16_t` values handed to `pushImage` as byte-swapped RGB565,
which is how images are stored. With `_LE` the drawing is correct but the
channels are crossed: yellow comes out magenta.

**The clock is drawn on the display, not on the canvas.** GIF frames are drawn
on top of previous ones, so a clock painted into the canvas would smear across
the animation.

## Clock

The Cardputer ADV has no RTC, so the time comes from NTP at boot. Copy
`src/secretos.ejemplo.h` to `src/secretos.h` and fill in your network; that file
is gitignored. Without it everything still works, just without the clock. The
radio is switched off as soon as it syncs.

## Build

```bash
pio run -e cardputer-adv     # firmware
pio test -e pruebas          # the playlist logic, runs on your computer
```

The playlist lives in `lib/nucleo/`, free of Arduino, so it can be tested on a
desktop. Everything hardware is confined to `src/`.

## Credits

Uses [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF) by Larry Bank and
[M5Unified](https://github.com/m5stack/M5Unified) / M5Cardputer by M5Stack.

MIT licensed. The wallpapers you put on your own card are yours to deal with:
none ship with this repository.
