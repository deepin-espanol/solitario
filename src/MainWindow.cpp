#include "MainWindow.h"
#include "TableroWidget.h"

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent) {
    resize(1024, 720);
    setWindowTitle(QStringLiteral("Solitario"));

    tablero = new TableroWidget(this);
    setCentralWidget(tablero);

    tablero->IniciarJuego();
}
