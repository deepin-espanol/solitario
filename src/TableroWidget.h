#ifndef TABLEROWIDGET_H
#define TABLEROWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include "Carta.h"
#include "Monton.h"
#include "JuegoKlondike.h"
#include "JuegoSpider.h"
#include "JuegoFreeCell.h"

class QSoundEffect;

struct ShuffleCard {
    int fromX, fromY;
    int toX, toY;
    float progress;
    Carta carta;
    int col;
    int idx;
};

class TableroWidget : public QWidget {
    Q_OBJECT
public:
    explicit TableroWidget(QWidget *parent = nullptr);

    void IniciarJuego(Juego tipo);
    bool IniciarJuegoFromJson(const QString &json);
    Juego TipoJuego() const { return tipoJuego; }
    Dificultad DificultadActual() const { return dificultad; }
    void SetDificultad(Dificultad d) { dificultad = d; }

    QString SerializarEstado() const;
    bool HayPartidaGuardada();
    bool HayPartida() const;

    int TiempoSegundos() const { return tiempoSegundos; }
    int Puntuacion() const { return puntuacion; }
    int Movimientos() const { return movimientos; }

    void SetFondoVerde(int variante);

signals:
    void Gano();
    void MovimientosChanged(int n);
    void TiempoChanged(int s);
    void PuntuacionChanged(int s);
    void PartidaGuardada();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void Deshacer();
    void Rehacer();
    void CancelarDrag();

private slots:
    void AnimarDrag();
    void TickTemporizador();
    void AnimarBarajado();

private:
    void DibujarCarta(QPainter &p, const Carta &c, int x, int y, int w, int h, float lift = 0.0f);
    void DibujarCartaBocaAbajo(QPainter &p, int x, int y, int w, int h, float lift = 0.0f);
    void DibujarSombra(QPainter &p, int x, int y, int w, int h, int blur, QColor c);
    void DibujarDropTarget(QPainter &p, int col);
    void DibujarFondo(QPainter &p);
    void DibujarKlondike(QPainter &p);
    void DibujarSpider(QPainter &p);
    void DibujarFreeCell(QPainter &p);
    void DibujarStockCount(QPainter &p, int count, int x, int y, int w, int h);

    int HitTest(int mx, int my, int &col, int &idx);
    void IniciarDrag(int mx, int my);
    bool HaGanado() const;
    bool TodasCartasBocaArriba();
    int ColumnaObjetivo(int mx, int my) const;
    bool ValidarDropEnColumna(int tgtCol);
    void AutoCompletar();
    void DespuesDeJugada();
    int WasteFan() const;
    int WasteTopOffset() const;
    void CalcularPuntuacion();
    void ReproducirSonido(const QString &tipo);

    QRect ColumnaRect(int col) const;
    int AnchoCarta() const;
    int AltoCarta() const;
    int EspaciadoX() const;
    int EspaciadoY() const;
    int FundacionX(int i) const;

    Juego tipoJuego = Juego::Klondike;
    Dificultad dificultad = Dificultad::Medio;
    JuegoKlondike klondike;
    JuegoSpider spider;
    JuegoFreeCell freecell;

    // Drag state
    bool dragging = false;
    int dragCol = -1;
    int dragIdx = -1;
    QVector<Carta> dragCartas;
    QPoint dragOrigen;
    QPoint dragMouseActual;
    QPoint dragMouseFinal;
    float dragLift = 0.0f;
    QTimer animTimer;
    bool snapeando = false;
    QPoint snapOrigen;
    QPoint snapDestino;
    float snapProgreso = 0.0f;

    int dropColResaltado = -1;
    bool dropValido = false;
    int movimientos = 0;

    // Timer & score
    int tiempoSegundos = 0;
    int puntuacion = 0;
    QTimer timerReloj;

    // Shuffle animation
    bool barajando = false;
    QVector<ShuffleCard> shuffleCards;
    QTimer shuffleTimer;

    // Background variant (0=classic, 1=dark, 2=light)
    int fondoVerde = 0;

    // Sound effects
    QSoundEffect *sndFlip = nullptr;
    QSoundEffect *sndMove = nullptr;
    QSoundEffect *sndWin = nullptr;
};

#endif
