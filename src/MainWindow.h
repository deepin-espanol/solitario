#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DMenu>

DWIDGET_USE_NAMESPACE

class TableroWidget;

class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void ActualizarTitulo();
    void GuardarPartida();
    void CargarPartida();

    TableroWidget *tablero;
    DMenu *menuJuego;
    Juego juegoActual = Juego::Klondike;
};

#endif