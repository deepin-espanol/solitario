#include "MainWindow.h"
#include "TableroWidget.h"
#include <DTitlebar>
#include <DWindowManagerHelper>
#include <QMessageBox>
#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QIcon>
#include <DAboutDialog>
#include <DApplication>
#include <QCloseEvent>
#include <QFile>
#include <QSaveFile>
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
    // Antes de la primera partida la mesa esta vacia: guardarla dejaba un
    // archivo que, si el juego se cerraba de golpe, impedia arrancar despues.
    if (!tablero->HayPartida()) return;
    // QSaveFile escribe en un temporal y lo sustituye al final: un cierre
    // inesperado nunca deja el archivo a medias.
    QSaveFile f(SavePath() + "/partida.json");
    if (f.open(QIODevice::WriteOnly)) {
        f.write(tablero->SerializarEstado().toUtf8());
        f.commit();
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
        // Partida danada o incompleta: se descarta y empieza una nueva
        if (!tablero->IniciarJuegoFromJson(data)) {
            QFile::remove(path);
            SeleccionarJuego(Juego::Klondike);
            return;
        }
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
    actNewGame->setShortcut(QKeySequence(QStringLiteral("Ctrl+N")));
    // Ctrl+Z / Ctrl+Shift+Z los gestiona el tablero; aqui solo como opcion de menu
    QAction *actDeshacer = menuJuego->addAction(tr("Deshacer (Ctrl+Z)"));
    QAction *actRehacer = menuJuego->addAction(tr("Rehacer (Ctrl+Shift+Z)"));
    connect(actDeshacer, &QAction::triggered, tablero, &TableroWidget::Deshacer);
    connect(actRehacer, &QAction::triggered, tablero, &TableroWidget::Rehacer);

    menuJuego->addSeparator();
    QMenu *menuDif = menuJuego->addMenu(tr("Dificultad"));
    QAction *actFacil = menuDif->addAction(tr("Fácil"));
    QAction *actMedio = menuDif->addAction(tr("Medio"));
    QAction *actDificil = menuDif->addAction(tr("Difícil"));
    // Grupos exclusivos: Qt gestiona las marcas sin capturar variables locales
    // por referencia (antes las lambdas apuntaban a listas ya destruidas).
    QSettings sIni;
    auto *grupoDif = new QActionGroup(this);
    grupoDif->setExclusive(true);
    const QList<QAction*> difs = {actFacil, actMedio, actDificil};
    const int savedDif = qBound(0, sIni.value("dificultad", 1).toInt(), 2);
    for (int i = 0; i < difs.size(); i++) {
        QAction *a = difs[i];
        a->setCheckable(true);
        a->setChecked(i == savedDif);
        grupoDif->addAction(a);
        connect(a, &QAction::triggered, this, [this, i]() {
            tablero->SetDificultad(static_cast<Dificultad>(i));
            QSettings s; s.setValue("dificultad", i);
            ActualizarTitulo();
        });
    }

    menuJuego->addSeparator();
    QMenu *menuFondo = menuJuego->addMenu(tr("Fondo de mesa"));
    QAction *actFondoClasico = menuFondo->addAction(tr("Clásico (verde)"));
    QAction *actFondoOscuro = menuFondo->addAction(tr("Oscuro"));
    QAction *actFondoClaro = menuFondo->addAction(tr("Claro"));
    auto *grupoFondo = new QActionGroup(this);
    grupoFondo->setExclusive(true);
    const QList<QAction*> fondos = {actFondoClasico, actFondoOscuro, actFondoClaro};
    const int savedFondo = qBound(0, sIni.value("fondo_verde", 0).toInt(), 2);
    for (int i = 0; i < fondos.size(); i++) {
        QAction *a = fondos[i];
        a->setCheckable(true);
        a->setChecked(i == savedFondo);
        grupoFondo->addAction(a);
        connect(a, &QAction::triggered, this, [this, i]() {
            tablero->SetFondoVerde(i);
            QSettings s; s.setValue("fondo_verde", i);
        });
    }

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
        dlg.setVersion(QApplication::applicationVersion());
        dlg.setDescription(
            "<div style='text-align:center;'><img src=':/Logo-Racoon.png' width='128'/></div>"
            + tr("Juego de cartas para DDE") + "<br/>"
            "<span style=' font-size:8pt; font-weight:600;'>Deepin en Español </span>"
            "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a>");
        dlg.setWebsiteName(QStringLiteral("deepinenespanol.org"));
        dlg.setWebsiteLink(QStringLiteral("https://deepinenespanol.org"));
        dlg.setProductIcon(QIcon(":/logo.svg"));
        dlg.exec();
    });

    menuJuego->addSeparator();
    QAction *actSalir = menuJuego->addAction(tr("Salir"));
    actSalir->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
    connect(actSalir, &QAction::triggered, this, &QWidget::close);

    // Los atajos del menu de la barra de titulo solo funcionan con el menu
    // abierto; anadirlas tambien a la ventana los activa siempre.
    addAction(actNewGame);
    addAction(actSalir);

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
