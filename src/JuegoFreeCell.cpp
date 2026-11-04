#include "JuegoFreeCell.h"
#include <algorithm>
#include <random>

void JuegoFreeCell::Iniciar(Dificultad dif) {
    celdasLibres.clear();
    columnas.clear();
    foundations.clear();
    historial.limpiar();

    int numCeldas = 4;
    switch (dif) {
    case Dificultad::Facil:  numCeldas = 6; break;
    case Dificultad::Medio:  numCeldas = 4; break;
    case Dificultad::Dificil: numCeldas = 2; break;
    }

    QVector<Carta> mazo;
    for (int p = 0; p < 4; p++)
        for (int v = 1; v <= 13; v++)
            mazo.append({static_cast<Palo>(p), v, true});

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(mazo.begin(), mazo.end(), g);

    columnas.resize(8);
    foundations.resize(4);
    celdasLibres.resize(numCeldas);

    for (int i = 0; i < mazo.size(); i++)
        columnas[i % 8].Agregar(mazo[i]);
}

EstadoMesa JuegoFreeCell::Capturar() const {
    EstadoMesa e;
    e.columnas = CopiaMontones(columnas);
    e.foundations = CopiaMontones(foundations);
    e.celdas = CopiaCartas(celdasLibres);
    return e;
}

void JuegoFreeCell::Restaurar(const EstadoMesa &e) {
    columnas = e.columnas;
    foundations = e.foundations;
    celdasLibres = e.celdas;
    if (columnas.size() != 8) columnas.resize(8);
    if (foundations.size() != 4) foundations.resize(4);
}

bool JuegoFreeCell::MoverACeldaLibre(int desdeCol, int desdeIdx) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    auto &origen = columnas[desdeCol];
    if (origen.Vacia() || desdeIdx != origen.Tamanio() - 1) return false;

    for (auto &c : celdasLibres) {
        if (c.EsVacia()) {
            historial.registrar(Capturar());
            c = origen.Quitar();
            return true;
        }
    }
    return false;
}

bool JuegoFreeCell::MoverAFoundation(int desdeCol, int desdeIdx) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    auto &origen = columnas[desdeCol];
    if (origen.Vacia() || desdeIdx != origen.Tamanio() - 1) return false;

    const int f = FundacionPara(origen.Top(), foundations);
    if (f < 0) return false;
    historial.registrar(Capturar());
    foundations[f].Agregar(origen.Quitar());
    return true;
}

// Cuantas cartas se pueden mover a la vez: (celdas libres + 1) * 2^columnas
// vacias. Si el destino es una columna vacia, esa no cuenta como auxiliar.
int JuegoFreeCell::MaxCartasMovibles(int aCol, int desdeCol) const {
    int freeCells = 0;
    for (const auto &c : celdasLibres)
        if (c.EsVacia()) freeCells++;
    int emptyCols = 0;
    for (int i = 0; i < columnas.size(); i++)
        if (i != desdeCol && i != aCol && columnas[i].Vacia()) emptyCols++;
    return (freeCells + 1) * (1 << emptyCols);
}

bool JuegoFreeCell::MoverAColumna(int desdeCol, int desdeIdx, int aCol) {
    if (desdeCol < 0 || desdeCol >= columnas.size()) return false;
    if (aCol < 0 || aCol >= columnas.size() || aCol == desdeCol) return false;

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

    if (cartas.size() > MaxCartasMovibles(aCol, desdeCol)) return false;

    auto &dest = columnas[aCol];
    if (!dest.Vacia()) {
        const Carta &top = dest.Top();
        if (top.EsRojo() == cartas.first().EsRojo()) return false;
        if (top.valor != cartas.first().valor + 1) return false;
    }

    historial.registrar(Capturar());
    origen.cartas.resize(desdeIdx);
    for (auto &c : cartas) dest.Agregar(c);
    return true;
}

bool JuegoFreeCell::HaGanado() const {
    for (const auto &f : foundations)
        if (f.Vacia() || f.Top().valor != 13) return false;
    return true;
}

bool JuegoFreeCell::MoverAColumnaDesdeCeldaLibre(int celda, int aCol) {
    if (celda < 0 || celda >= celdasLibres.size()) return false;
    if (aCol < 0 || aCol >= columnas.size()) return false;
    if (celdasLibres[celda].EsVacia()) return false;

    const Carta carta = celdasLibres[celda];
    auto &dest = columnas[aCol];
    if (!dest.Vacia()) {
        if (dest.Top().EsRojo() == carta.EsRojo()) return false;
        if (dest.Top().valor != carta.valor + 1) return false;
    }

    historial.registrar(Capturar());
    celdasLibres[celda] = Carta{};
    dest.Agregar(carta);
    return true;
}

bool JuegoFreeCell::MoverAFoundationDesdeCeldaLibre(int celda) {
    if (celda < 0 || celda >= celdasLibres.size()) return false;
    if (celdasLibres[celda].EsVacia()) return false;

    const Carta carta = celdasLibres[celda];
    const int f = FundacionPara(carta, foundations);
    if (f < 0) return false;
    historial.registrar(Capturar());
    celdasLibres[celda] = Carta{};
    foundations[f].Agregar(carta);
    return true;
}

bool JuegoFreeCell::MoverACeldaLibreDesdeCeldaLibre(int desdeCelda, int aCelda) {
    if (desdeCelda < 0 || desdeCelda >= celdasLibres.size()) return false;
    if (aCelda < 0 || aCelda >= celdasLibres.size()) return false;
    if (desdeCelda == aCelda) return false;
    if (celdasLibres[desdeCelda].EsVacia()) return false;
    if (!celdasLibres[aCelda].EsVacia()) return false;

    historial.registrar(Capturar());
    celdasLibres[aCelda] = celdasLibres[desdeCelda];
    celdasLibres[desdeCelda] = Carta{};
    return true;
}

// Sube cartas a la fundacion como parte de la jugada que lo provoca (no
// registra historial propio: deshacer revierte jugada + subidas juntas).
bool JuegoFreeCell::autoFoundation(bool soloSeguras) {
    bool algo = false;
    bool moved = true;
    while (moved) {
        moved = false;
        for (int i = 0; i < celdasLibres.size() && !moved; i++) {
            if (celdasLibres[i].EsVacia()) continue;
            const Carta c = celdasLibres[i];
            const int f = FundacionPara(c, foundations);
            if (f >= 0 && (!soloSeguras || EsSeguraParaFundacion(c, foundations))) {
                celdasLibres[i] = Carta{};
                foundations[f].Agregar(c);
                moved = algo = true;
            }
        }
        for (int i = 0; i < columnas.size() && !moved; i++) {
            if (columnas[i].Vacia()) continue;
            const Carta c = columnas[i].Top();
            const int f = FundacionPara(c, foundations);
            if (f >= 0 && (!soloSeguras || EsSeguraParaFundacion(c, foundations))) {
                columnas[i].Quitar();
                foundations[f].Agregar(c);
                moved = algo = true;
            }
        }
    }
    return algo;
}

// Si cada columna esta en orden descendente, la carta mas baja que queda
// siempre esta accesible y puede subir: la partida se termina sola.
bool JuegoFreeCell::ResolubleAutomaticamente() const {
    for (const auto &col : columnas)
        for (int i = 1; i < col.Tamanio(); i++)
            if (col.cartas[i].valor >= col.cartas[i - 1].valor)
                return false;
    return true;
}

bool JuegoFreeCell::Deshacer() {
    EstadoMesa e;
    if (!historial.deshacer(Capturar(), e)) return false;
    Restaurar(e);
    return true;
}

bool JuegoFreeCell::Rehacer() {
    EstadoMesa e;
    if (!historial.rehacer(Capturar(), e)) return false;
    Restaurar(e);
    return true;
}
