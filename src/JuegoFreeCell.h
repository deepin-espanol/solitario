#ifndef JUEGOFREECELL_H
#define JUEGOFREECELL_H

#include "Carta.h"
#include "Monton.h"
#include <QVector>

class JuegoFreeCell {
public:
    void Iniciar(Dificultad dif = Dificultad::Medio);
    bool MoverACeldaLibre(int desdeCol, int desdeIdx);
    bool MoverAFoundation(int desdeCol, int desdeIdx);
    bool MoverAColumna(int desdeCol, int desdeIdx, int aCol);
    bool HaGanado() const;
    bool MoverAColumnaDesdeCeldaLibre(int celda, int aCol);
    bool MoverAFoundationDesdeCeldaLibre(int celda);
    bool MoverACeldaLibreDesdeCeldaLibre(int desdeCelda, int aCelda);
    bool autoFoundation();
    bool Deshacer();
    bool Rehacer();

    QVector<Carta> &CeldasLibres() { return celdasLibres; }
    QVector<Monton> &Columnas() { return columnas; }
    QVector<Monton> &Foundations() { return foundations; }
    const QVector<Carta> &CeldasLibres() const { return celdasLibres; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const QVector<Monton> &Foundations() const { return foundations; }
    const QVector<Accion> &Historial() const { return historial; }
    const QVector<Accion> &RedoHistorial() const { return redoHistorial; }
    void SetHistorial(const QVector<Accion> &h) { historial = h; }
    void SetRedoHistorial(const QVector<Accion> &h) { redoHistorial = h; }

private:
    QVector<Carta> celdasLibres;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    QVector<Accion> historial;
    QVector<Accion> redoHistorial;
};

#endif