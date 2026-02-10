#ifndef TABLEROWIDGET_H
#define TABLEROWIDGET_H

#include <QWidget>
#include <QVector>
#include "Carta.h"
#include "Monton.h"
#include "JuegoKlondike.h"

class TableroWidget : public QWidget {
    Q_OBJECT
public:
    explicit TableroWidget(QWidget *parent = nullptr);

    void IniciarJuego();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void DibujarCarta(QPainter &p, const Carta &c, int x, int y, int w, int h);
    void DibujarCartaBocaAbajo(QPainter &p, int x, int y, int w, int h);
    void DibujarSombra(QPainter &p, int x, int y, int w, int h, int blur, QColor c);
    void DibujarFondo(QPainter &p);
    void DibujarKlondike(QPainter &p);
    void DibujarStockCount(QPainter &p, int count, int x, int y, int w, int h);

    int AnchoCarta() const;
    int AltoCarta() const;
    int EspaciadoX() const;
    int EspaciadoY() const;

    JuegoKlondike klondike;
    int fondoVerde = 0;
};

#endif