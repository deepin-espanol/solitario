#include "MainWindow.h"

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent) {
    resize(1024, 720);
    setWindowTitle(QStringLiteral("Solitario"));
}
