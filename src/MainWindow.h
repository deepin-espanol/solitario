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

private:
    void ActualizarTitulo();

    TableroWidget *tablero;
    DMenu *menuJuego;
};

#endif