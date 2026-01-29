#include "JuegoKlondike.h"
#include <algorithm>
#include <random>

void JuegoKlondike::Iniciar(Dificultad dif) {
    stock.Limpiar();
    waste.Limpiar();
    columnas.clear();
    foundations.clear();

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
        return true;
    }
    Carta c = stock.Quitar();
    c.bocaArriba = true;
    waste.Agregar(c);
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
                    moved = true;
                    break;
                } else if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1) {
                    waste.Quitar();
                    f.Agregar(c);
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

bool JuegoKlondike::HaGanado() const {
    for (const auto &f : foundations) {
        if (f.Vacia() || f.Top().valor != 13) return false;
    }
    return true;
}
