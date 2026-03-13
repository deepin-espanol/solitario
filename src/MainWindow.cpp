#include "MainWindow.h"
#include "TableroWidget.h"
#include <QMessageBox>
#include <QTimer>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent) {
    resize(1024, 720);
    setWindowTitle(QStringLiteral("Solitario"));

    tablero = new TableroWidget(this);
    setCentralWidget(tablero);

    connect(tablero, &TableroWidget::MovimientosChanged, this, [this](int) {
        ActualizarTitulo();
    });

    connect(tablero, &TableroWidget::TiempoChanged, this, [this](int) {
        ActualizarTitulo();
    });

    connect(tablero, &TableroWidget::PuntuacionChanged, this, [this](int) {
        ActualizarTitulo();
    });

    connect(tablero, &TableroWidget::Gano, this, [this]() {
        QTimer::singleShot(100, this, [this]() {
            QMessageBox::information(this,
                tr("Felicidades"),
                tr("Has ganado el juego!"));
        });
    });

    tablero->IniciarJuego();
    ActualizarTitulo();
}

void MainWindow::ActualizarTitulo() {
    int t = tablero->TiempoSegundos();
    int m = t / 60;
    int s = t % 60;
    QString tiempo = QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));

    int moves = tablero->Movimientos();
    int score = tablero->Puntuacion();

    titlebar()->setTitle(tr("Solitario  |  %1  |  %2 mov.  |  %3 pts.")
        .arg(tiempo).arg(moves).arg(score));
}
