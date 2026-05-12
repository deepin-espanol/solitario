#include "MainWindow.h"
#include "TableroWidget.h"
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

DWIDGET_USE_NAMESPACE

static QString SavePath() {
    QString p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(p);
    return p;
}

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

    CargarPartida();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    GuardarPartida();
    event->accept();
}

void MainWindow::GuardarPartida() {
    QString path = SavePath() + "/partida.json";
    QString data = tablero->SerializarEstado();
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(data.toUtf8());
        f.close();
    }
}

void MainWindow::CargarPartida() {
    QString path = SavePath() + "/partida.json";
    QFile f(path);
    if (!f.exists()) {
        tablero->IniciarJuego(Juego::Klondike);
        return;
    }
    if (f.open(QIODevice::ReadOnly)) {
        QString data = QString::fromUtf8(f.readAll());
        f.close();
        tablero->IniciarJuegoFromJson(data);
        juegoActual = tablero->TipoJuego();
        ActualizarTitulo();
    } else {
        tablero->IniciarJuego(Juego::Klondike);
    }
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
