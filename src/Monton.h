#ifndef MONTON_H
#define MONTON_H

#include "Carta.h"
#include <QVector>

struct Monton {
    QVector<Carta> cartas;

    void Agregar(const Carta &c) { cartas.append(c); }
    Carta Quitar() { if (cartas.isEmpty()) return Carta{}; return cartas.takeLast(); }
    Carta &Top() { return cartas.last(); }
    const Carta &Top() const { return cartas.last(); }
    bool Vacia() const { return cartas.isEmpty(); }
    int Tamanio() const { return cartas.size(); }
    void Limpiar() { cartas.clear(); }
};

#endif
