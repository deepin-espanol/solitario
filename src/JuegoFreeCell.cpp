#include "JuegoFreeCell.h"
#include <algorithm>
#include <random>

void JuegoFreeCell::Iniciar(Dificultad dif) {
    celdasLibres.clear();
    columnas.clear();
    foundations.clear();
    historial.clear();
    redoHistorial.clear();

    int numCeldas;
    switch (dif) {
    case Dificultad::Facil:  numCeldas = 6; break;
    case Dificultad::Medio:  numCeldas = 4; break;
    case Dificultad::Dificil: numCeldas = 2; break;
    }

    QVector<Carta> mazo;
    for (int p = 0; p < 4; p++) {
        for (int v = 1; v <= 13; v++) {
            mazo.append({static_cast<Palo>(p), v, true});
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(mazo.begin(), mazo.end(), g);

    columnas.resize(8);
    foundations.resize(4);
    celdasLibres.resize(numCeldas);

    for (int i = 0; i < mazo.size(); i++) {
        columnas[i % 8].Agregar(mazo[i]);
    }
}

bool JuegoFreeCell::MoverACeldaLibre(int desdeCol, int desdeIdx) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    auto &origen = columnas[desdeCol];
    if (desdeIdx != origen.Tamanio() - 1) return false;

    for (auto &c : celdasLibres) {
        if (c.EsVacia()) {
            c = origen.Quitar();
            return true;
        }
    }
    return false;
}

bool JuegoFreeCell::MoverAFoundation(int desdeCol, int desdeIdx) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    auto &origen = columnas[desdeCol];
    if (desdeIdx != origen.Tamanio() - 1) return false;

    Carta carta = origen.Quitar();

    for (auto &f : foundations) {
        if (f.Vacia() && carta.valor == 1) {
            f.Agregar(carta);
            return true;
        } else if (!f.Vacia() && f.Top().palo == carta.palo && f.Top().valor == carta.valor - 1) {
            f.Agregar(carta);
            return true;
        }
    }

    origen.Agregar(carta);
    return false;
}

bool JuegoFreeCell::MoverAColumna(int desdeCol, int desdeIdx, int aCol) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    if (aCol < 0 || aCol >= columnas.size()) return false;

    auto &origen = columnas[desdeCol];
    if (desdeIdx < 0 || desdeIdx >= origen.Tamanio()) return false;

    QVector<Carta> cartas;
    for (int i = desdeIdx; i < origen.Tamanio(); i++) {
        if (i > desdeIdx) {
            if (cartas.last().EsRojo() == origen.cartas[i].EsRojo()) return false;
            if (origen.cartas[i].valor != cartas.last().valor - 1) return false;
        }
        cartas.append(origen.cartas[i]);
    }

    // Check if we have enough free cells + empty columns
    int freeCells = 0;
    for (const auto &c : celdasLibres) {
        if (c.EsVacia()) freeCells++;
    }
    int emptyCols = 0;
    for (int i = 0; i < columnas.size(); i++) {
        if (i == desdeCol) continue;
        if (columnas[i].Vacia()) emptyCols++;
    }
    if (cartas.size() > (1LL << emptyCols) * (freeCells + 1)) return false;

    auto &dest = columnas[aCol];
    if (!dest.Vacia()) {
        const Carta &top = dest.Top();
        if (top.EsRojo() == cartas.first().EsRojo()) return false;
        if (top.valor != cartas.first().valor + 1) return false;
    }

    origen.cartas.resize(desdeIdx);
    for (auto &c : cartas) dest.Agregar(c);

    historial.append({Accion::MoverColumnas, desdeCol, desdeIdx, aCol, (int)cartas.size(), false});
    redoHistorial.clear();
    return true;
}

bool JuegoFreeCell::HaGanado() const {
    for (const auto &f : foundations) {
        if (f.Vacia() || f.Top().valor != 13) return false;
    }
    return true;
}

bool JuegoFreeCell::MoverAColumnaDesdeCeldaLibre(int celda, int aCol) {
    if (celda < 0 || celda >= celdasLibres.size()) return false;
    if (aCol < 0 || aCol >= columnas.size()) return false;
    if (celdasLibres[celda].EsVacia()) return false;

    Carta carta = celdasLibres[celda];

    auto &dest = columnas[aCol];
    if (!dest.Vacia()) {
        if (dest.Top().EsRojo() == carta.EsRojo()) return false;
        if (dest.Top().valor != carta.valor + 1) return false;
    }

    celdasLibres[celda] = Carta{};
    dest.Agregar(carta);
    return true;
}

bool JuegoFreeCell::MoverAFoundationDesdeCeldaLibre(int celda) {
    if (celda < 0 || celda >= celdasLibres.size()) return false;
    if (celdasLibres[celda].EsVacia()) return false;

    Carta carta = celdasLibres[celda];

    for (auto &f : foundations) {
        if (f.Vacia() && carta.valor == 1) {
            celdasLibres[celda] = Carta{};
            f.Agregar(carta);
            return true;
        } else if (!f.Vacia() && f.Top().palo == carta.palo && f.Top().valor == carta.valor - 1) {
            celdasLibres[celda] = Carta{};
            f.Agregar(carta);
            return true;
        }
    }
    return false;
}

bool JuegoFreeCell::MoverACeldaLibreDesdeCeldaLibre(int desdeCelda, int aCelda) {
    if (desdeCelda < 0 || desdeCelda >= celdasLibres.size()) return false;
    if (aCelda < 0 || aCelda >= celdasLibres.size()) return false;
    if (desdeCelda == aCelda) return false;
    if (celdasLibres[desdeCelda].EsVacia()) return false;
    if (!celdasLibres[aCelda].EsVacia()) return false;

    celdasLibres[aCelda] = celdasLibres[desdeCelda];
    celdasLibres[desdeCelda] = Carta{};
    return true;
}

bool JuegoFreeCell::autoFoundation() {
    bool moved = true;
    while (moved) {
        moved = false;
        for (int i = 0; i < celdasLibres.size(); i++) {
            if (celdasLibres[i].EsVacia()) continue;
            const Carta &c = celdasLibres[i];
            for (auto &f : foundations) {
                if (f.Vacia() && c.valor == 1) {
                    celdasLibres[i] = Carta{};
                    f.Agregar(c);
                    moved = true;
                    break;
                } else if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1) {
                    celdasLibres[i] = Carta{};
                    f.Agregar(c);
                    moved = true;
                    break;
                }
            }
            if (moved) break;
        }
        if (moved) continue;
        for (int i = 0; i < columnas.size(); i++) {
            if (columnas[i].Vacia()) continue;
            const Carta &c = columnas[i].Top();
            for (auto &f : foundations) {
                if (f.Vacia() && c.valor == 1) {
                    columnas[i].Quitar();
                    f.Agregar(c);
                    moved = true;
                    break;
                } else if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1) {
                    columnas[i].Quitar();
                    f.Agregar(c);
                    moved = true;
                    break;
                }
            }
            if (moved) break;
        }
    }
    return true;
}

bool JuegoFreeCell::Deshacer() {
    if (historial.isEmpty()) return false;
    Accion a = historial.takeLast();
    redoHistorial.append(a);

    if (a.tipo == Accion::MoverColumnas) {
        auto &dest = columnas[a.aCol];
        QVector<Carta> cartas;
        for (int i = 0; i < a.cartasMovidas; i++) cartas.prepend(dest.Quitar());
        for (auto &c : cartas) columnas[a.desdeCol].Agregar(c);
    }
    return true;
}

bool JuegoFreeCell::Rehacer() {
    if (redoHistorial.isEmpty()) return false;
    Accion a = redoHistorial.takeLast();
    historial.append(a);

    if (a.tipo == Accion::MoverColumnas) {
        auto &origen = columnas[a.desdeCol];
        int start = origen.Tamanio() - a.cartasMovidas;
        QVector<Carta> cartas;
        for (int i = start; i < origen.Tamanio(); i++) cartas.append(origen.cartas[i]);
        origen.cartas.resize(start);
        for (auto &c : cartas) columnas[a.aCol].Agregar(c);
    }
    return true;
}
