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
    bool Deshacer();
    bool Rehacer();
    bool HayDeshacer() const { return !historial.isEmpty(); }
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
    const QVector<Accion> &Historial() const { return historial; }
    const QVector<Accion> &RedoHistorial() const { return redoHistorial; }
    void SetHistorial(const QVector<Accion> &h) { historial = h; }
    void SetRedoHistorial(const QVector<Accion> &h) { redoHistorial = h; }

private:
    void GuardarAccion(Accion::Tipo tipo, int desdeCol, int desdeIdx, int aCol, int cartasMovidas, bool volteoAuto = false);

    Monton stock;
    Monton waste;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    QVector<Accion> historial;
    QVector<Accion> redoHistorial;
};

#endif
