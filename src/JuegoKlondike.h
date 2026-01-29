#ifndef JUEGOKLONDIKE_H
#define JUEGOKLONDIKE_H

#include "Carta.h"
#include "Monton.h"
#include <QVector>

class JuegoKlondike {
public:
    void Iniciar(Dificultad dif = Dificultad::Medio);
    bool MoverAColumna(int desdeCol, int desdeIdx, int aCol);
    bool MoverAFoundation(int desdeCol, int desdeIdx);
    bool voltear(int col);
    bool sacarDeStock();
    bool autoFoundation();
    bool HaGanado() const;
    QVector<Carta> &Stock() { return stock.cartas; }
    QVector<Carta> &Waste() { return waste.cartas; }
    QVector<Monton> &Columnas() { return columnas; }
    QVector<Monton> &Foundations() { return foundations; }
    Monton &StockMonton() { return stock; }
    Monton &WasteMonton() { return waste; }
    const QVector<Carta> &Stock() const { return stock.cartas; }
    const QVector<Carta> &Waste() const { return waste.cartas; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const QVector<Monton> &Foundations() const { return foundations; }
    const Monton &StockMonton() const { return stock; }
    const Monton &WasteMonton() const { return waste; }

private:
    Monton stock;
    Monton waste;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
};

#endif