#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DMenu>
#include <DApplication>

#include "Carta.h"

DWIDGET_USE_NAMESPACE

class TableroWidget;

class MainWindow : public DMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void CrearMenu();
    void SeleccionarJuego(Juego tipo);
    void ActualizarTitulo();
    void GuardarPartida();
    void CargarPartida();
    void MostrarEstadisticas();

    TableroWidget *tablero;
    DMenu *menuJuego;
    Juego juegoActual = Juego::Klondike;
    bool pantallaCompleta = false;
};

#endif