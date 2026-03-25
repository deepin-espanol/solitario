#include "JuegoSpider.h"
#include <algorithm>
#include <random>

void JuegoSpider::Iniciar(Dificultad dif) {
    stock.Limpiar();
    columnas.clear();
    foundations.clear();

    int palos;
    switch (dif) {
    case Dificultad::Facil:  palos = 1; break;
    case Dificultad::Medio:  palos = 2; break;
    case Dificultad::Dificil: palos = 4; break;
    }
    palosUsados = palos;

    int decenasPorPalo = 8 / palos;

    QVector<Carta> mazo;
    for (int p = 0; p < palos; p++) {
        for (int deck = 0; deck < decenasPorPalo; deck++) {
            for (int v = 1; v <= 13; v++) {
                mazo.append({static_cast<Palo>(p), v, false});
            }
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(mazo.begin(), mazo.end(), g);

    columnas.resize(10);
    foundations.resize(8);

    int idx = 0;
    for (int i = 0; i < 10; i++) {
        int count = (i < 4) ? 6 : 5;
        for (int j = 0; j < count; j++) {
            Carta c = mazo[idx++];
            c.bocaArriba = (j == count - 1);
            columnas[i].Agregar(c);
        }
    }
    while (idx < mazo.size()) {
        Carta c = mazo[idx++];
        c.bocaArriba = false;
        stock.Agregar(c);
    }
}

bool JuegoSpider::MoverAColumna(int desdeCol, int desdeIdx, int aCol) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    if (aCol < 0 || aCol >= columnas.size()) return false;
    if (desdeCol == aCol) return false;

    auto &origen = columnas[desdeCol];
    if (desdeIdx < 0 || desdeIdx >= origen.Tamanio()) return false;

    QVector<Carta> cartas;
    for (int i = desdeIdx; i < origen.Tamanio(); i++) {
        if (!origen.cartas[i].bocaArriba) return false;
        if (i > desdeIdx) {
            if (origen.cartas[i].valor != origen.cartas[i-1].valor - 1) return false;
            if (origen.cartas[i].palo != origen.cartas[desdeIdx].palo) return false;
        }
        cartas.append(origen.cartas[i]);
    }

    auto &dest = columnas[aCol];
    if (!dest.Vacia()) {
        if (dest.Top().valor != cartas.first().valor + 1) return false;
    }

    origen.cartas.resize(desdeIdx);
    for (auto &c : cartas) dest.Agregar(c);

    if (!origen.Vacia() && !origen.Top().bocaArriba) {
        origen.Top().bocaArriba = true;
    }

    return true;
}

bool JuegoSpider::CompletarSecuencia(int col) {
    if (col < 0 || col >= columnas.size()) return false;
    auto &c = columnas[col];
    if (c.cartas.size() < 13) return false;

    int baseIdx = c.cartas.size() - 13;
    int base = c.cartas[baseIdx].valor;
    Palo paloBase = c.cartas[baseIdx].palo;

    if (base != 1) return false;

    for (int i = baseIdx; i < c.cartas.size(); i++) {
        if (!c.cartas[i].bocaArriba) return false;
        if (c.cartas[i].valor != i - baseIdx + 1) return false;
        if (c.cartas[i].palo != paloBase) return false;
    }

    for (int i = 0; i < 13; i++) c.Quitar();

    for (auto &f : foundations) {
        if (f.Vacia()) {
            Carta fc = {paloBase, 13, true};
            f.Agregar(fc);
            break;
        }
    }

    if (!c.Vacia() && !c.Top().bocaArriba) {
        c.Top().bocaArriba = true;
    }

    return true;
}

bool JuegoSpider::sacarDeStock() {
    if (stock.Tamanio() < 10) return false;

    for (int i = 0; i < 10; i++) {
        Carta c = stock.Quitar();
        c.bocaArriba = true;
        columnas[i].Agregar(c);
    }
    return true;
}

bool JuegoSpider::HaGanado() const {
    return stock.Vacia() &&
           std::all_of(foundations.begin(), foundations.end(),
                        [](const Monton &f) { return !f.Vacia(); });
}