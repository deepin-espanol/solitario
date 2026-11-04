#ifndef JUEGOSPIDER_H
#define JUEGOSPIDER_H

#include "Carta.h"
#include "Monton.h"
#include "Historial.h"
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
    Monton &StockMonton() { return stock; }
    QVector<Monton> &Foundations() { return foundations; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const Monton &StockMonton() const { return stock; }
    const QVector<Monton> &Foundations() const { return foundations; }
    Historial &HistorialRef() { return historial; }
    const Historial &HistorialRef() const { return historial; }

    EstadoMesa Capturar() const;
    void Restaurar(const EstadoMesa &e);

private:
    Monton stock;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    int palosUsados = 4;
    Historial historial;
};

#endif
