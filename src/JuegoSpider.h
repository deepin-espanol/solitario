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
    bool Deshacer();
    bool Rehacer();
    bool HaGanado() const;
    QVector<Monton> &Columnas() { return columnas; }
    QVector<Monton> &Foundations() { return foundations; }
    Monton &StockMonton() { return stock; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const QVector<Monton> &Foundations() const { return foundations; }
    const Monton &StockMonton() const { return stock; }
    const QVector<Accion> &Historial() const { return historial; }
    const QVector<Accion> &RedoHistorial() const { return redoHistorial; }
    void SetHistorial(const QVector<Accion> &h) { historial = h; }
    void SetRedoHistorial(const QVector<Accion> &h) { redoHistorial = h; }

private:
    Monton stock;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    QVector<Accion> historial;
    QVector<Accion> redoHistorial;
    int palosUsados = 1;
};

#endif