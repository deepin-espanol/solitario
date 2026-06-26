#include "JuegoKlondike.h"
#include <algorithm>
#include <random>

void JuegoKlondike::Iniciar(Dificultad dif) {
    stock.Limpiar();
    waste.Limpiar();
    columnas.clear();
    foundations.clear();
    historial.clear();
    redoHistorial.clear();

    QVector<Carta> mazo;
    for (int p = 0; p < 4; p++) {
        for (int v = 1; v <= 13; v++) {
            mazo.append({static_cast<Palo>(p), v, false});
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(mazo.begin(), mazo.end(), g);

    columnas.resize(7);
    foundations.resize(4);

    int idx = 0;
    for (int i = 0; i < 7; i++) {
        for (int j = i; j < 7; j++) {
            Carta c = mazo[idx++];
            c.bocaArriba = (j == i);
            columnas[j].Agregar(c);
        }
    }
    while (idx < 52) {
        stock.Agregar(mazo[idx++]);
    }

    if (dif == Dificultad::Facil) {
        for (int i = 0; i < 7; i++) {
            if (columnas[i].Vacia()) continue;
            const Carta &top = columnas[i].Top();
            if (top.bocaArriba && (top.valor == 1 || top.valor == 2)) continue;
            for (int j = 0; j < i; j++) {
                if (columnas[j].Vacia()) continue;
                const Carta &other = columnas[j].Top();
                if (other.bocaArriba && (other.valor == 1 || other.valor == 2)) {
                    auto tmp = top;
                    columnas[i].cartas.last() = other;
                    columnas[j].cartas.last() = tmp;
                    break;
                }
            }
        }
    }
}

void JuegoKlondike::GuardarAccion(Accion::Tipo tipo, int desdeCol, int desdeIdx, int aCol, int cartasMovidas, bool volteoAuto) {
    historial.append({tipo, desdeCol, desdeIdx, aCol, cartasMovidas, volteoAuto});
    redoHistorial.clear();
}

bool JuegoKlondike::MoverAColumna(int desdeCol, int desdeIdx, int aCol) {
    if (desdeCol < -1 || desdeCol >= (int)columnas.size() || aCol < 0 || aCol >= (int)columnas.size())
        return false;

    QVector<Carta> cartas;
    if (desdeCol == -1) {
        if (waste.Vacia()) return false;
        cartas.append(waste.Top());
    } else {
        auto &origen = columnas[desdeCol];
        if (desdeIdx < 0 || desdeIdx >= origen.Tamanio()) return false;
        for (int i = desdeIdx; i < origen.Tamanio(); i++) {
            if (!origen.cartas[i].bocaArriba) return false;
            cartas.append(origen.cartas[i]);
        }
    }

    auto &dest = columnas[aCol];
    if (!dest.Vacia()) {
        const Carta &top = dest.Top();
        const Carta &first = cartas.first();
        if (top.EsRojo() == first.EsRojo()) return false;
        if (top.valor != first.valor + 1) return false;
    } else {
        if (cartas.first().valor != 13) return false;
    }

    GuardarAccion(Accion::MoverColumnas, desdeCol, desdeIdx, aCol, (int)cartas.size());

    if (desdeCol == -1) {
        waste.Quitar();
    } else {
        columnas[desdeCol].cartas.resize(desdeIdx);
    }

    for (auto &c : cartas) dest.Agregar(c);

    if (desdeCol >= 0 && !columnas[desdeCol].Vacia() && !columnas[desdeCol].Top().bocaArriba) {
        columnas[desdeCol].Top().bocaArriba = true;
    }

    return true;
}

bool JuegoKlondike::MoverAFoundation(int desdeCol, int desdeIdx) {
    if (desdeCol < -1 || desdeCol >= (int)columnas.size()) return false;

    Carta c;
    if (desdeCol == -1) {
        if (waste.Vacia()) return false;
        c = waste.Top();
    } else {
        auto &origen = columnas[desdeCol];
        if (desdeIdx != origen.Tamanio() - 1) return false;
        if (!origen.Top().bocaArriba) return false;
        c = origen.Top();
    }

    int foundIdx = -1;
    for (int i = 0; i < foundations.size(); i++) {
        auto &f = foundations[i];
        if (f.Vacia() && c.valor == 1) { foundIdx = i; break; }
        else if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1) { foundIdx = i; break; }
    }
    if (foundIdx < 0) return false;

    if (desdeCol == -1) {
        waste.Quitar();
    } else {
        columnas[desdeCol].Quitar();
    }
    foundations[foundIdx].Agregar(c);

    GuardarAccion(Accion::Foundation, desdeCol, desdeIdx, foundIdx, 1);

    if (desdeCol >= 0 && !columnas[desdeCol].Vacia() && !columnas[desdeCol].Top().bocaArriba) {
        columnas[desdeCol].Top().bocaArriba = true;
    }

    return true;
}

bool JuegoKlondike::sacarDeStock() {
    if (stock.Vacia()) {
        if (waste.Vacia()) return false;
        QVector<Carta> temp = waste.cartas;
        for (auto &c : temp) c.bocaArriba = false;
        std::reverse(temp.begin(), temp.end());
        stock.cartas = temp;
        waste.Limpiar();
        GuardarAccion(Accion::StockDraw, -1, temp.size(), -1, 0);
        return true;
    }
    Carta c = stock.Quitar();
    c.bocaArriba = true;
    waste.Agregar(c);
    GuardarAccion(Accion::StockDraw, -1, 1, -1, 1);
    return true;
}

bool JuegoKlondike::autoFoundation() {
    bool moved = true;
    while (moved) {
        moved = false;
        if (!waste.Vacia()) {
            const Carta &c = waste.Top();
            for (int i = 0; i < foundations.size(); i++) {
                auto &f = foundations[i];
                if (f.Vacia() && c.valor == 1) {
                    waste.Quitar();
                    f.Agregar(c);
                    GuardarAccion(Accion::Foundation, -1, 0, i, 1);
                    moved = true;
                    break;
                } else if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1) {
                    waste.Quitar();
                    f.Agregar(c);
                    GuardarAccion(Accion::Foundation, -1, 0, i, 1);
                    moved = true;
                    break;
                }
            }
        }
        for (int i = 0; i < columnas.size(); i++) {
            if (columnas[i].Vacia()) continue;
            if (!columnas[i].Top().bocaArriba) {
                columnas[i].Top().bocaArriba = true;
                moved = true;
                break;
            }
            const Carta &c = columnas[i].Top();
            int origIdx = columnas[i].Tamanio() - 1;
            for (int fi = 0; fi < foundations.size(); fi++) {
                auto &f = foundations[fi];
                if (f.Vacia() && c.valor == 1) {
                    columnas[i].Quitar();
                    f.Agregar(c);
                    GuardarAccion(Accion::Foundation, i, origIdx, fi, 1);
                    moved = true;
                    break;
                } else if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1) {
                    columnas[i].Quitar();
                    f.Agregar(c);
                    GuardarAccion(Accion::Foundation, i, origIdx, fi, 1);
                    moved = true;
                    break;
                }
            }
            if (moved) break;
        }
    }
    return true;
}

bool JuegoKlondike::Deshacer() {
    if (historial.isEmpty()) return false;

    Accion a = historial.takeLast();
    redoHistorial.append(a);

    switch (a.tipo) {
    case Accion::MoverColumnas: {
        auto &dest = columnas[a.aCol];
        QVector<Carta> cartas;
        for (int i = 0; i < a.cartasMovidas; i++) cartas.prepend(dest.Quitar());
        if (a.desdeCol == -1) {
            waste.Agregar(cartas.first());
        } else {
            for (auto &c : cartas) columnas[a.desdeCol].Agregar(c);
        }
        break;
    }
    case Accion::Foundation: {
        Carta c = foundations[a.aCol].Quitar();
        if (a.desdeCol == -1) {
            waste.Agregar(c);
        } else {
            columnas[a.desdeCol].Agregar(c);
        }
        break;
    }
    case Accion::StockDraw: {
        if (a.cartasMovidas == 0) {
            // Stock was recycled: restore waste from stock
            std::reverse(stock.cartas.begin(), stock.cartas.end());
            for (auto &c : stock.cartas) c.bocaArriba = true;
            waste.cartas = stock.cartas;
            stock.Limpiar();
        } else {
            // Single card draw: return card from waste back to stock face-down
            Carta c = waste.Quitar();
            c.bocaArriba = false;
            stock.Agregar(c);
        }
        break;
    }
    case Accion::AutoFlip:
        if (a.desdeCol >= 0 && a.desdeCol < columnas.size()) {
            if (!columnas[a.desdeCol].Vacia() && columnas[a.desdeCol].Top().bocaArriba) {
                columnas[a.desdeCol].Top().bocaArriba = false;
            }
        }
        break;
    }
    return true;
}

bool JuegoKlondike::Rehacer() {
    if (redoHistorial.isEmpty()) return false;
    Accion a = redoHistorial.takeLast();
    historial.append(a);

    switch (a.tipo) {
    case Accion::MoverColumnas: {
        QVector<Carta> cartas;
        if (a.desdeCol == -1) {
            if (waste.Vacia()) return false;
            cartas.append(waste.Quitar());
        } else {
            auto &origen = columnas[a.desdeCol];
            int start = origen.Tamanio() - a.cartasMovidas;
            for (int i = start; i < origen.Tamanio(); i++) {
                cartas.append(origen.cartas[i]);
            }
            origen.cartas.resize(start);
        }
        for (auto &c : cartas) columnas[a.aCol].Agregar(c);
        break;
    }
    case Accion::Foundation: {
        Carta c;
        if (a.desdeCol == -1) {
            if (waste.Vacia()) return false;
            c = waste.Quitar();
        } else {
            c = columnas[a.desdeCol].Quitar();
        }
        foundations[a.aCol].Agregar(c);
        break;
    }
    case Accion::StockDraw: {
        if (a.cartasMovidas == 0) {
            // Recycle waste back to stock
            QVector<Carta> temp = waste.cartas;
            for (auto &c : temp) c.bocaArriba = false;
            std::reverse(temp.begin(), temp.end());
            stock.cartas = temp;
            waste.Limpiar();
        } else {
            // Single card draw: move from stock to waste
            Carta c = stock.Quitar();
            c.bocaArriba = true;
            waste.Agregar(c);
        }
        break;
    }
    case Accion::AutoFlip:
        if (a.desdeCol >= 0 && !columnas[a.desdeCol].Vacia()) {
            columnas[a.desdeCol].Top().bocaArriba = true;
        }
        break;
    }
    return true;
}

bool JuegoKlondike::HaGanado() const {
    for (const auto &f : foundations) {
        if (f.Vacia() || f.Top().valor != 13) return false;
    }
    return true;
}