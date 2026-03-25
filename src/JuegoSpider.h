#ifndef JUEGOSPIDER_H
#define JUEGOSPIDER_H

#include "Carta.h"
#include "Monton.h"
#include <QVector>

class JuegoSpider {
public:
    void Iniciar(Dificultad dif = Dificultad::Medio);
    bool MoverAColumna(int desdeCol, int desdeIdx, int aCol);
    bool CompletarSecuencia(int col);
    bool sacarDeStock();
    bool HaGanado() const;
    QVector<Monton> &Columnas() { return columnas; }
    QVector<Monton> &Foundations() { return foundations; }
    Monton &StockMonton() { return stock; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const QVector<Monton> &Foundations() const { return foundations; }
    const Monton &StockMonton() const { return stock; }

private:
    Monton stock;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    int palosUsados = 1;
};

#endif