#include "JuegoKlondike.h"
#include <algorithm>
#include <random>

void JuegoKlondike::Iniciar(Dificultad dif) {
    stock.Limpiar();
    waste.Limpiar();
    columnas.clear();
    foundations.clear();
    historial.limpiar();

    QVector<Carta> mazo;
    for (int p = 0; p < 4; p++)
        for (int v = 1; v <= 13; v++)
            mazo.append({static_cast<Palo>(p), v, false});

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
    while (idx < 52)
        stock.Agregar(mazo[idx++]);

    // Dificultad: Facil y Medio roban de una en una; Dificil roba de tres en tres
    robo = (dif == Dificultad::Dificil) ? 3 : 1;

    // Facil: deja Ases y doses a la vista en las columnas cuando es posible
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

EstadoMesa JuegoKlondike::Capturar() const {
    EstadoMesa e;
    e.stock = CopiaMonton(stock);
    e.waste = CopiaMonton(waste);
    e.columnas = CopiaMontones(columnas);
    e.foundations = CopiaMontones(foundations);
    return e;
}

void JuegoKlondike::Restaurar(const EstadoMesa &e) {
    stock = e.stock;
    waste = e.waste;
    columnas = e.columnas;
    foundations = e.foundations;
    if (columnas.size() != 7) columnas.resize(7);
    if (foundations.size() != 4) foundations.resize(4);
}

void JuegoKlondike::VoltearSuperior(int col) {
    if (col >= 0 && col < columnas.size() && !columnas[col].Vacia() && !columnas[col].Top().bocaArriba)
        columnas[col].Top().bocaArriba = true;
}

bool JuegoKlondike::MoverAColumna(int desdeCol, int desdeIdx, int aCol) {
    if (desdeCol < -1 || desdeCol >= (int)columnas.size() || aCol < 0 || aCol >= (int)columnas.size())
        return false;
    if (desdeCol == aCol) return false;

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
    } else if (cartas.first().valor != 13) {
        return false;
    }

    historial.registrar(Capturar());

    if (desdeCol == -1)
        waste.Quitar();
    else
        columnas[desdeCol].cartas.resize(desdeIdx);

    for (auto &c : cartas) dest.Agregar(c);
    VoltearSuperior(desdeCol);
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
        if (origen.Vacia() || desdeIdx != origen.Tamanio() - 1) return false;
        if (!origen.Top().bocaArriba) return false;
        c = origen.Top();
    }

    const int foundIdx = FundacionPara(c, foundations);
    if (foundIdx < 0) return false;

    historial.registrar(Capturar());
    if (desdeCol == -1)
        waste.Quitar();
    else
        columnas[desdeCol].Quitar();
    foundations[foundIdx].Agregar(c);
    VoltearSuperior(desdeCol);
    return true;
}

bool JuegoKlondike::sacarDeStock() {
    if (stock.Vacia()) {
        // Mazo agotado: el descarte vuelve boca abajo al mazo
        if (waste.Vacia()) return false;
        historial.registrar(Capturar());
        QVector<Carta> temp = waste.cartas;
        for (auto &c : temp) c.bocaArriba = false;
        std::reverse(temp.begin(), temp.end());
        stock.cartas = temp;
        waste.Limpiar();
        return true;
    }
    historial.registrar(Capturar());
    for (int i = 0; i < robo && !stock.Vacia(); ++i) {
        Carta c = stock.Quitar();
        c.bocaArriba = true;
        waste.Agregar(c);
    }
    return true;
}

// Sube cartas a la fundacion. Forma parte de la jugada que lo provoca, asi
// que no registra historial propio: deshacer revierte jugada + subidas juntas.
bool JuegoKlondike::autoFoundation(bool soloSeguras) {
    bool algo = false;
    bool moved = true;
    while (moved) {
        moved = false;
        for (int i = 0; i < columnas.size(); i++)
            VoltearSuperior(i);

        if (!waste.Vacia()) {
            const Carta c = waste.Top();
            const int f = FundacionPara(c, foundations);
            if (f >= 0 && (!soloSeguras || EsSeguraParaFundacion(c, foundations))) {
                waste.Quitar();
                foundations[f].Agregar(c);
                moved = algo = true;
                continue;
            }
        }
        for (int i = 0; i < columnas.size() && !moved; i++) {
            if (columnas[i].Vacia()) continue;
            const Carta c = columnas[i].Top();
            const int f = FundacionPara(c, foundations);
            if (f >= 0 && (!soloSeguras || EsSeguraParaFundacion(c, foundations))) {
                columnas[i].Quitar();
                foundations[f].Agregar(c);
                VoltearSuperior(i);
                moved = algo = true;
            }
        }
    }
    return algo;
}

bool JuegoKlondike::Deshacer() {
    EstadoMesa e;
    if (!historial.deshacer(Capturar(), e)) return false;
    Restaurar(e);
    return true;
}

bool JuegoKlondike::Rehacer() {
    EstadoMesa e;
    if (!historial.rehacer(Capturar(), e)) return false;
    Restaurar(e);
    return true;
}

bool JuegoKlondike::HaGanado() const {
    for (const auto &f : foundations)
        if (f.Vacia() || f.Top().valor != 13) return false;
    return true;
}
