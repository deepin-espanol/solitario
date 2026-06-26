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
    bool MoverAColumnaDesdeCeldaLibre(int celda, int aCol);
    bool MoverAFoundationDesdeCeldaLibre(int celda);
    bool MoverACeldaLibreDesdeCeldaLibre(int desdeCelda, int aCelda);
    bool autoFoundation();
    bool Deshacer();
    bool Rehacer();
    bool HaGanado() const;
    QVector<Monton> &Columnas() { return columnas; }
    QVector<Carta> &CeldasLibres() { return celdasLibres; }
    QVector<Monton> &Foundations() { return foundations; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const QVector<Carta> &CeldasLibres() const { return celdasLibres; }
    const QVector<Monton> &Foundations() const { return foundations; }
    const QVector<Accion> &Historial() const { return historial; }
    const QVector<Accion> &RedoHistorial() const { return redoHistorial; }
    void SetHistorial(const QVector<Accion> &h) { historial = h; }
    void SetRedoHistorial(const QVector<Accion> &h) { redoHistorial = h; }

private:
    QVector<Carta> celdasLibres; // 4 free cells
    QVector<Monton> columnas;   // 8 columns
    QVector<Monton> foundations; // 4 foundations
    QVector<Accion> historial;
    QVector<Accion> redoHistorial;
};

#endif
