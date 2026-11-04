#ifndef JUEGOFREECELL_H
#define JUEGOFREECELL_H

#include "Carta.h"
#include "Monton.h"
#include "Historial.h"
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
    // soloSeguras: solo sube cartas que ya no hacen falta en el tablero
    bool autoFoundation(bool soloSeguras = true);
    // Todas las columnas en orden descendente: se puede terminar automaticamente
    bool ResolubleAutomaticamente() const;
    int MaxCartasMovibles(int aCol, int desdeCol) const;
    bool Deshacer();
    bool Rehacer();
    bool HaGanado() const;
    QVector<Monton> &Columnas() { return columnas; }
    QVector<Carta> &CeldasLibres() { return celdasLibres; }
    QVector<Monton> &Foundations() { return foundations; }
    const QVector<Monton> &Columnas() const { return columnas; }
    const QVector<Carta> &CeldasLibres() const { return celdasLibres; }
    const QVector<Monton> &Foundations() const { return foundations; }
    Historial &HistorialRef() { return historial; }
    const Historial &HistorialRef() const { return historial; }

    EstadoMesa Capturar() const;
    void Restaurar(const EstadoMesa &e);

private:
    QVector<Carta> celdasLibres; // 4 free cells
    QVector<Monton> columnas;   // 8 columns
    QVector<Monton> foundations; // 4 foundations
    Historial historial;
};

#endif
