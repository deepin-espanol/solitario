#ifndef JUEGOKLONDIKE_H
#define JUEGOKLONDIKE_H

#include "Carta.h"
#include "Monton.h"
#include "Historial.h"
#include <QVector>

class JuegoKlondike {
public:
    void Iniciar(Dificultad dif = Dificultad::Medio);
    bool MoverAColumna(int desdeCol, int desdeIdx, int aCol);
    bool MoverAFoundation(int desdeCol, int desdeIdx);
    bool sacarDeStock();
    // soloSeguras: solo sube cartas que ya no hacen falta en el tablero
    bool autoFoundation(bool soloSeguras = true);
    bool Deshacer();
    bool Rehacer();
    bool HayDeshacer() const { return historial.hayDeshacer(); }
    bool HaGanado() const;
    int CartasPorRobo() const { return robo; }

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
    Historial &HistorialRef() { return historial; }
    const Historial &HistorialRef() const { return historial; }
    void SetCartasPorRobo(int n) { robo = (n == 3) ? 3 : 1; }

    EstadoMesa Capturar() const;
    void Restaurar(const EstadoMesa &e);

private:
    void VoltearSuperior(int col);

    Monton stock;
    Monton waste;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    Historial historial;
    int robo = 1;  // cartas que se pasan del mazo al descarte en cada robo
};

#endif
