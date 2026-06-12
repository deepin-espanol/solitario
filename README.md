# Solitario

Juego de cartas tipo solitario para Deepin, desarrollado con Qt6 y Deepin Tool Kit (DTK6).

## Variantes

- **Klondike** - El clásico solitario con 7 columnas, stock y 4 fundaciones
- **Spider** - Spider Solitaire con 10 columnas y secuencias completas de K a A
- **FreeCell** - 4 celdas libres, 8 columnas, todas las cartas boca arriba

## Dependencias

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev libdtk6widget-dev cmake
```

## Compilar

```bash
mkdir build && cd build
cmake ..
make
```

## Ejecutar

```bash
./solitario
```

## Empaquetar

```bash
make -f debian/rules binary
```
