# Solitario

Juego de cartas tipo solitario con 3 variantes, desarrollado con Qt6 y Deepin Tool Kit (DTK6).

## Variantes

- **Klondike** - El clásico solitario con 7 columnas, stock y 4 fundaciones
- **Spider** - Spider Solitaire con 10 columnas y secuencias completas de K a A
- **FreeCell** - 4 celdas libres, 8 columnas, todas las cartas boca arriba

## Controles

- **Click izquierdo** en stock: voltear carta (Klondike) / repartir (Spider)
- **Arrastrar y soltar** para mover cartas entre columnas
- **Click en carta** de la columna más alta para auto-mover a fundación (Klondike)

## Menú

- `Juego > Klondike / Spider / FreeCell` - Cambiar variante
- `Juego > Nuevo juego` - Reiniciar variante actual
- `Juego > Dificultad` / `Fondo de mesa` - Se recuerdan entre sesiones
- `Juego > Estadísticas` - Partidas jugadas y ganadas por variante
- `F11` - Pantalla completa

## Paquete .deb

```bash
sudo apt build-dep ./
dpkg-buildpackage -us -uc -b
sudo apt install ../solitario_*.deb
```

## Dependencias

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev qt6-l10n-tools libdtk6widget-dev cmake g++
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

O si se instalo globalmente:

```bash
sudo make install
solitario
```

## Solucionar errores comunes

Si el compilador falla con `-fuse-linker-plugin` en Deepin:

```bash
cp -r /usr/lib/gcc/x86_64-linux-gnu/12/* /tmp/lib/gcc/x86_64-linux-gnu/12/
```

## Estructura

```
Solitario/
├── CMakeLists.txt
├── README.md
└── src/
    ├── main.cpp          - Punto de entrada, inicializa DTK
    ├── Carta.h           - Modelo de carta (palo, valor)
    ├── Monton.h          - Pila de cartas
    ├── JuegoKlondike.*   - Lógica de Klondike
    ├── JuegoSpider.*     - Lógica de Spider
    ├── JuegoFreeCell.*   - Lógica de FreeCell
    ├── TableroWidget.*   - Widget de renderizado e interacción
    └── MainWindow.*      - Ventana principal DTK
```
