#!/usr/bin/env python3
"""Ajusta cualquier GIF a los 240x135 de la pantalla del Cardputer.

Por qué hace falta: el firmware sabe reducir por divisores enteros, pero el
ESP32 igual tiene que decodificar el cuadro completo antes de tirar píxeles. Un
GIF de 640x360 son 230.000 píxeles por cuadro para mostrar 32.000. Convertirlo
antes divide el trabajo por nueve y de paso achica el archivo muchísimo.

    python3 herramientas/ajustar.py entrada.gif salida.gif [--caber]
    python3 herramientas/ajustar.py carpeta/ /Volumes/CARDPUTER/caratulas

Acepta un archivo o una carpeta entera. Con una carpeta convierte todos los
.gif que encuentre y los deja en el destino, así agregar carátulas nuevas es
un solo comando.

Por defecto **recorta** al centro para llenar la pantalla. Con `--caber` encaja
la imagen entera y deja barras negras, que sirve cuando recortar se comería
algo importante.

Con `--sprite` trata la imagen como una animación sobre fondo plano, de esas
con croma verde o un color liso: detecta ese fondo, lo pinta de negro, y
recorta a donde de verdad pasa algo. Sin eso quedan personajes diminutos en una
esquina de un rectángulo de color.

Necesita Pillow:  ~/.venvs/imagen/bin/python
"""

import pathlib
import sys

from PIL import Image, ImageSequence

ANCHO, ALTO = 240, 135


def ajustar_cuadro(cuadro, recortar):
    im = cuadro.convert("RGB")
    if recortar:
        # Recorte centrado a 16:9 y después reducción.
        objetivo = ANCHO / ALTO
        a, al = im.size
        if a / al > objetivo:                 # más ancha: sobra a los lados
            nuevo = int(round(al * objetivo))
            izq = (a - nuevo) // 2
            im = im.crop((izq, 0, izq + nuevo, al))
        else:                                  # más alta: sobra arriba y abajo
            nuevo = int(round(a / objetivo))
            arriba = (al - nuevo) // 2
            im = im.crop((0, arriba, a, arriba + nuevo))
        return im.resize((ANCHO, ALTO), Image.LANCZOS)

    # Modo caber: la imagen entera, centrada sobre negro.
    copia = im.copy()
    copia.thumbnail((ANCHO, ALTO), Image.LANCZOS)
    lienzo = Image.new("RGB", (ANCHO, ALTO), (0, 0, 0))
    lienzo.paste(copia, ((ANCHO - copia.width) // 2, (ALTO - copia.height) // 2))
    return lienzo


def color_de_fondo(cuadros):
    """El color que más se repite en las cuatro esquinas de todos los cuadros."""
    conteo = {}
    for im in cuadros:
        a, al = im.size
        for x, y in ((0, 0), (a - 1, 0), (0, al - 1), (a - 1, al - 1)):
            c = im.getpixel((x, y))
            conteo[c] = conteo.get(c, 0) + 1
    return max(conteo, key=conteo.get) if conteo else (0, 0, 0)


def caja_del_contenido(cuadros, fondo, tolerancia=40):
    """Lo que ocupa la acción en TODOS los cuadros juntos.

    Tiene que ser una sola caja para toda la animación: si cada cuadro se
    recortara a su medida, el personaje saltaría de posición.
    """
    a, al = cuadros[0].size
    x0, y0, x1, y1 = a, al, -1, -1
    for im in cuadros:
        px = im.load()
        for y in range(al):
            for x in range(a):
                r, g, b = px[x, y][:3]
                if (abs(r - fondo[0]) + abs(g - fondo[1]) + abs(b - fondo[2])) > tolerancia:
                    if x < x0: x0 = x
                    if x > x1: x1 = x
                    if y < y0: y0 = y
                    if y > y1: y1 = y
    if x1 < 0:
        return 0, 0, a - 1, al - 1
    return x0, y0, x1, y1


def preparar_sprite(cuadros):
    """Fondo plano a negro y recorte a donde pasa algo."""
    fondo = color_de_fondo(cuadros)
    x0, y0, x1, y1 = caja_del_contenido(cuadros, fondo)

    # La caja se estira a 16:9 alrededor de su centro, para no deformar nada.
    ancho, alto = x1 - x0 + 1, y1 - y0 + 1
    objetivo = ANCHO / ALTO
    if ancho / alto < objetivo:
        nuevo = int(round(alto * objetivo))
        x0 -= (nuevo - ancho) // 2
        x1 = x0 + nuevo - 1
    else:
        nuevo = int(round(ancho / objetivo))
        y0 -= (nuevo - alto) // 2
        y1 = y0 + nuevo - 1

    salida = []
    for im in cuadros:
        negro = Image.new("RGB", im.size, (0, 0, 0))
        px, pn = im.load(), negro.load()
        a, al = im.size
        for y in range(al):
            for x in range(a):
                r, g, b = px[x, y][:3]
                if (abs(r - fondo[0]) + abs(g - fondo[1]) + abs(b - fondo[2])) > 40:
                    pn[x, y] = (r, g, b)
        recorte = negro.crop((x0, y0, x1 + 1, y1 + 1))
        lienzo = Image.new("RGB", (ANCHO, ALTO), (0, 0, 0))
        copia = recorte.resize((ANCHO, ALTO), Image.LANCZOS)
        lienzo.paste(copia, (0, 0))
        salida.append(lienzo)
    print(f"      fondo detectado {fondo}, recortado a {x1-x0+1}x{y1-y0+1}")
    return salida


def convertir(origen, destino, recortar=True, sprite=False):
    im = Image.open(origen)
    cuadros, duraciones = [], []
    for cuadro in ImageSequence.Iterator(im):
        cuadros.append(cuadro.convert("RGB") if sprite
                       else ajustar_cuadro(cuadro, recortar))
        # Un GIF sin duración declarada se reproduce a unos 10 cuadros por
        # segundo, que es lo que hacen los navegadores.
        duraciones.append(cuadro.info.get("duration", 100) or 100)

    if not cuadros:
        raise SystemExit(f"{origen}: no tiene ningún cuadro")

    if sprite:
        cuadros = preparar_sprite(cuadros)

    cuadros[0].save(
        destino,
        save_all=True,
        append_images=cuadros[1:],
        duration=duraciones,
        loop=0,
        optimize=True,
        disposal=2,          # cada cuadro parte de cero: más simple de decodificar
    )

    antes = pathlib.Path(origen).stat().st_size
    despues = pathlib.Path(destino).stat().st_size
    print(f"  {pathlib.Path(origen).name}")
    print(f"      {im.size[0]}x{im.size[1]} -> {ANCHO}x{ALTO}, "
          f"{len(cuadros)} cuadros, {antes // 1024} KB -> {despues // 1024} KB")
    return despues


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) != 2:
        raise SystemExit(__doc__)

    recortar = "--caber" not in sys.argv
    sprite = "--sprite" in sys.argv
    origen = pathlib.Path(args[0])
    destino = pathlib.Path(args[1])

    if origen.is_file():
        convertir(origen, destino, recortar, sprite)
        return

    if not origen.is_dir():
        raise SystemExit(f"{origen}: no existe")

    destino.mkdir(parents=True, exist_ok=True)
    gifs = sorted(p for p in origen.iterdir()
                  if p.suffix.lower() == ".gif" and not p.name.startswith("."))
    if not gifs:
        raise SystemExit(f"{origen}: no hay ningún .gif")

    total = 0
    for gif in gifs:
        salida = destino / gif.name
        try:
            total += convertir(gif, salida, recortar, sprite)
        except Exception as e:                      # un GIF roto no para el resto
            print(f"  {gif.name}: se saltó, {e}")
    print(f"\n{len(gifs)} archivos, {total // 1024} KB en total")


if __name__ == "__main__":
    main()
