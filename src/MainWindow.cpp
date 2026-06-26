#include "MainWindow.h"
#include "TableroWidget.h"
#include <DTitlebar>
#include <DWindowManagerHelper>
#include <QMessageBox>
#include <QAction>
#include <QTimer>
#include <QIcon>
#include <DAboutDialog>
#include <DApplication>
#include <QCloseEvent>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QKeySequence>
#include <QPushButton>

DWIDGET_USE_NAMESPACE

static QString SavePath() {
    QString p = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(p);
    return p;
}

MainWindow::MainWindow(QWidget *parent) : DMainWindow(parent) {
    resize(1024, 720);
    setWindowIcon(QIcon(":/logo.svg"));

    tablero = new TableroWidget(this);
    setCentralWidget(tablero);

    CrearMenu();

    QSettings s;
    int savedFondo = s.value("fondo_verde", 0).toInt();
    tablero->SetFondoVerde(savedFondo);
    int savedDif = s.value("dificultad", 1).toInt();
    tablero->SetDificultad(static_cast<Dificultad>(savedDif));

    CargarPartida();

    DWindowManagerHelper::instance()->setMotifFunctions(window()->windowHandle(),
        DWindowManagerHelper::FUNC_MINIMIZE |
        DWindowManagerHelper::FUNC_MAXIMIZE |
        DWindowManagerHelper::FUNC_CLOSE |
        DWindowManagerHelper::FUNC_RESIZE |
        DWindowManagerHelper::FUNC_MOVE);

    connect(tablero, &TableroWidget::Gano, this, [this]() {
        QSettings s;
        QString key = "stats_" + QString::number(static_cast<int>(juegoActual));
        QVariantMap m = s.value(key).toMap();
        m["won"] = m["won"].toInt() + 1;
        s.setValue(key, m);

        QTimer::singleShot(100, this, [this]() {
            QMessageBox::information(this,
                tr("Felicidades"),
                tr("Has ganado el juego!"));
        });
    });

    connect(tablero, &TableroWidget::MovimientosChanged, this, [this](int n) {
        ActualizarTitulo();
    });

    connect(tablero, &TableroWidget::TiempoChanged, this, [this](int) {
        ActualizarTitulo();
    });
}

void MainWindow::closeEvent(QCloseEvent *event) {
    GuardarPartida();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_F11) {
        if (pantallaCompleta) {
            showNormal();
            pantallaCompleta = false;
        } else {
            showFullScreen();
            pantallaCompleta = true;
        }
        return;
    }
    DMainWindow::keyPressEvent(event);
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
        SeleccionarJuego(Juego::Klondike);
        return;
    }
    if (f.open(QIODevice::ReadOnly)) {
        QString data = QString::fromUtf8(f.readAll());
        f.close();
        tablero->IniciarJuegoFromJson(data);
        juegoActual = tablero->TipoJuego();
        ActualizarTitulo();
    } else {
        SeleccionarJuego(Juego::Klondike);
    }
}

void MainWindow::MostrarEstadisticas() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Estadísticas"));
    dlg.setMinimumWidth(400);

    auto *lay = new QVBoxLayout(&dlg);

    QLabel *title = new QLabel(tr("<h2>Estadísticas</h2>"));
    title->setAlignment(Qt::AlignCenter);
    lay->addWidget(title);

    QTableWidget *table = new QTableWidget(3, 3);
    table->setHorizontalHeaderLabels({tr("Juego"), tr("Jugadas"), tr("Ganadas")});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QTableWidget::NoEditTriggers);

    struct Info { QString nombre; Juego tipo; };
    Info juegos[] = {
        {QStringLiteral("Klondike"), Juego::Klondike},
        {QStringLiteral("Spider"), Juego::Spider},
        {QStringLiteral("FreeCell"), Juego::FreeCell},
    };

    QSettings s;
    for (int i = 0; i < 3; i++) {
        QString key = "stats_" + QString::number(static_cast<int>(juegos[i].tipo));
        QVariantMap m = s.value(key).toMap();
        table->setItem(i, 0, new QTableWidgetItem(juegos[i].nombre));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(m["played"].toInt())));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(m["won"].toInt())));
    }

    lay->addWidget(table);

    QPushButton *closeBtn = new QPushButton(tr("Cerrar"));
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

void MainWindow::CrearMenu() {
    menuJuego = new DMenu(this);

    QAction *actKlondike = menuJuego->addAction(tr("Klondike (Clásico)"));
    QAction *actSpider = menuJuego->addAction(QStringLiteral("Spider"));
    QAction *actFreeCell = menuJuego->addAction(QStringLiteral("FreeCell"));

    menuJuego->addSeparator();
    QAction *actNewGame = menuJuego->addAction(tr("Nuevo juego"));

    menuJuego->addSeparator();
    QMenu *menuDif = menuJuego->addMenu(tr("Dificultad"));
    QAction *actFacil = menuDif->addAction(tr("Fácil"));
    QAction *actMedio = menuDif->addAction(tr("Medio"));
    QAction *actDificil = menuDif->addAction(tr("Difícil"));
    QList<QAction*> difs = {actFacil, actMedio, actDificil};
    for (QAction *a : difs) a->setCheckable(true);
    difs[1]->setChecked(true);

    QSettings sIni;
    int savedDif = sIni.value("dificultad", 1).toInt();
    for (int i = 0; i < difs.size(); i++) difs[i]->setChecked(i == savedDif);

    auto updateDifChecks = [&](int idx) {
        for (int i = 0; i < difs.size(); i++) difs[i]->setChecked(i == idx);
    };

    connect(actFacil, &QAction::triggered, this, [this, updateDifChecks]() {
        tablero->SetDificultad(Dificultad::Facil);
        QSettings s; s.setValue("dificultad", 0);
        updateDifChecks(0);
    });
    connect(actMedio, &QAction::triggered, this, [this, updateDifChecks]() {
        tablero->SetDificultad(Dificultad::Medio);
        QSettings s; s.setValue("dificultad", 1);
        updateDifChecks(1);
    });
    connect(actDificil, &QAction::triggered, this, [this, updateDifChecks]() {
        tablero->SetDificultad(Dificultad::Dificil);
        QSettings s; s.setValue("dificultad", 2);
        updateDifChecks(2);
    });

    menuJuego->addSeparator();
    QMenu *menuFondo = menuJuego->addMenu(tr("Fondo de mesa"));
    QAction *actFondoClasico = menuFondo->addAction(tr("Clásico (verde)"));
    QAction *actFondoOscuro = menuFondo->addAction(tr("Oscuro"));
    QAction *actFondoClaro = menuFondo->addAction(tr("Claro"));
    QList<QAction*> fondos = {actFondoClasico, actFondoOscuro, actFondoClaro};
    for (QAction *a : fondos) a->setCheckable(true);
    fondos[0]->setChecked(true);

    auto updateFondoChecks = [&](int idx) {
        for (int i = 0; i < fondos.size(); i++) fondos[i]->setChecked(i == idx);
    };

    connect(actFondoClasico, &QAction::triggered, this, [this, updateFondoChecks]() {
        tablero->SetFondoVerde(0);
        QSettings s; s.setValue("fondo_verde", 0);
        updateFondoChecks(0);
    });
    connect(actFondoOscuro, &QAction::triggered, this, [this, updateFondoChecks]() {
        tablero->SetFondoVerde(1);
        QSettings s; s.setValue("fondo_verde", 1);
        updateFondoChecks(1);
    });
    connect(actFondoClaro, &QAction::triggered, this, [this, updateFondoChecks]() {
        tablero->SetFondoVerde(2);
        QSettings s; s.setValue("fondo_verde", 2);
        updateFondoChecks(2);
    });

    menuJuego->addSeparator();
    QAction *actStats = menuJuego->addAction(tr("Estadísticas"));
    connect(actStats, &QAction::triggered, this, &MainWindow::MostrarEstadisticas);

    titlebar()->setMenu(menuJuego);

    connect(actKlondike, &QAction::triggered, this, [this]() { SeleccionarJuego(Juego::Klondike); });
    connect(actSpider, &QAction::triggered, this, [this]() { SeleccionarJuego(Juego::Spider); });
    connect(actFreeCell, &QAction::triggered, this, [this]() { SeleccionarJuego(Juego::FreeCell); });
    connect(actNewGame, &QAction::triggered, this, [this]() { SeleccionarJuego(tablero->TipoJuego()); });

    menuJuego->addSeparator();
    QAction *actAbout = menuJuego->addAction(tr("Acerca de"));

    connect(actAbout, &QAction::triggered, this, [this]() {
        DAboutDialog dlg(this);
        dlg.setProductName(QStringLiteral("Solitario"));
        dlg.setVersion(QStringLiteral("1.0.0"));
        dlg.setDescription(tr("Juego de cartas para DDE"));
        dlg.setProductIcon(QIcon(":/logo.svg"));
        dlg.exec();
    });

    QTimer::singleShot(0, this, [this]() {
        QMenu *m = titlebar()->menu();
        if (!m) return;
        for (QAction *a : m->actions()) {
            if (a->text() == DApplication::translate("TitleBarMenu", "About")) {
                a->setVisible(false);
                break;
            }
        }
    });
}

void MainWindow::ActualizarTitulo() {
    QString nombre;
    switch (juegoActual) {
    case Juego::Klondike: nombre = QStringLiteral("Klondike"); break;
    case Juego::Spider:   nombre = QStringLiteral("Spider");   break;
    case Juego::FreeCell: nombre = QStringLiteral("FreeCell"); break;
    }

    QString dif;
    switch (tablero->DificultadActual()) {
    case Dificultad::Facil:  dif = tr("Fácil"); break;
    case Dificultad::Medio:  dif = tr("Medio"); break;
    case Dificultad::Dificil: dif = tr("Difícil"); break;
    }

    int t = tablero->TiempoSegundos();
    int m = t / 60;
    int s = t % 60;
    QString tiempo = QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));

    int moves = tablero->Movimientos();
    int score = tablero->Puntuacion();

    titlebar()->setTitle(tr("Solitario - %1 [%2]  |  %3  |  %4 mov.  |  %5 pts.")
        .arg(nombre, dif, tiempo).arg(moves).arg(score));
}

void MainWindow::SeleccionarJuego(Juego tipo) {
    GuardarPartida();
    juegoActual = tipo;

    QSettings s;
    QString key = "stats_" + QString::number(static_cast<int>(tipo));
    QVariantMap stats = s.value(key).toMap();
    stats["played"] = stats["played"].toInt() + 1;
    s.setValue(key, stats);

    tablero->IniciarJuego(tipo);
    ActualizarTitulo();
}
