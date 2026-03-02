#ifndef TABLEROWIDGET_H
#define TABLEROWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include "Carta.h"
#include "Monton.h"
#include "JuegoKlondike.h"

class TableroWidget : public QWidget {
    Q_OBJECT
public:
    explicit TableroWidget(QWidget *parent = nullptr);

    void IniciarJuego();

    int TiempoSegundos() const { return tiempoSegundos; }
    int Puntuacion() const { return puntuacion; }
    int Movimientos() const { return movimientos; }

signals:
    void MovimientosChanged(int n);
    void TiempoChanged(int s);
    void PuntuacionChanged(int s);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void TickTemporizador();

private:
    void CalcularPuntuacion();
    void DibujarCarta(QPainter &p, const Carta &c, int x, int y, int w, int h);
    void DibujarCartaBocaAbajo(QPainter &p, int x, int y, int w, int h);
    void DibujarSombra(QPainter &p, int x, int y, int w, int h, int blur, QColor c);
    void DibujarDropTarget(QPainter &p, int col);
    void DibujarFondo(QPainter &p);
    void DibujarKlondike(QPainter &p);
    void DibujarStockCount(QPainter &p, int count, int x, int y, int w, int h);

    int HitTest(int mx, int my, int &col, int &idx);
    void IniciarDrag(int mx, int my);
    int ColumnaObjetivo(int mx, int my) const;
    bool ValidarDropEnColumna(int tgtCol);

    QRect ColumnaRect(int col) const;
    int AnchoCarta() const;
    int AltoCarta() const;
    int EspaciadoX() const;
    int EspaciadoY() const;

    JuegoKlondike klondike;

    // Drag state
    bool dragging = false;
    int dragCol = -1;
    int dragIdx = -1;
    QVector<Carta> dragCartas;
    QPoint dragMouseActual;

    int dropColResaltado = -1;
    bool dropValido = false;
    int movimientos = 0;

    // Timer & score
    int tiempoSegundos = 0;
    int puntuacion = 0;
    QTimer timerReloj;

    int fondoVerde = 0;
};

#endif