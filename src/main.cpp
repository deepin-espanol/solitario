#include <DApplication>
#include <DWidgetUtil>
#include <DMainWindow>
#include <QCommandLineParser>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include "MainWindow.h"

DWIDGET_USE_NAMESPACE

// Hasta la 1.0.3 la organizacion era "ghost" (el usuario del desarrollador):
// ajustes en ~/.config/ghost/solitario.conf y partida en
// ~/.local/share/ghost/solitario/. Se copian una sola vez a la nueva ubicacion.
static void migrarDatosAntiguos()
{
    QSettings nuevos;
    if (nuevos.value(QStringLiteral("migrado_desde_ghost"), false).toBool())
        return;

    QSettings antiguos(QStringLiteral("ghost"), QStringLiteral("solitario"));
    for (const QString &clave : antiguos.allKeys())
        if (!nuevos.contains(clave))
            nuevos.setValue(clave, antiguos.value(clave));

    const QString dirNuevo = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString dirAntiguo = dirNuevo;
    dirAntiguo.replace(QStringLiteral("/deepinenespanol/"), QStringLiteral("/ghost/"));
    const QString partidaAntigua = dirAntiguo + QStringLiteral("/partida.json");
    const QString partidaNueva = dirNuevo + QStringLiteral("/partida.json");
    if (QFile::exists(partidaAntigua) && !QFile::exists(partidaNueva)) {
        QDir().mkpath(dirNuevo);
        QFile::copy(partidaAntigua, partidaNueva);
    }
    nuevos.setValue(QStringLiteral("migrado_desde_ghost"), true);
}

int main(int argc, char *argv[]) {
    DApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("deepinenespanol"));
    app.setApplicationName(QStringLiteral("solitario"));
    app.setApplicationDisplayName(QStringLiteral("Solitario"));
    app.setApplicationVersion(QStringLiteral("1.0.3"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("solitario"), QIcon(QStringLiteral(":/logo.svg"))));
    migrarDatosAntiguos();

    QTranslator sysTranslator;
    QString locale = QLocale::system().name();
    if (sysTranslator.load(QStringLiteral("solitario_") + locale, QStringLiteral(":/i18n"))) {
        app.installTranslator(&sysTranslator);
    }

    app.setStyle("chameleon");

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Juego de Solitario"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    MainWindow w;
    w.setMinimumSize(800, 600);
    w.resize(1024, 720);
    moveToCenter(&w);
    w.show();

    return app.exec();
}
