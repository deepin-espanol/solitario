#include <DApplication>
#include <DWidgetUtil>
#include <DMainWindow>
#include <QCommandLineParser>
#include <QIcon>
#include "MainWindow.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[]) {
    DApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("ghost"));
    app.setApplicationName(QStringLiteral("solitario"));
    app.setApplicationDisplayName(QStringLiteral("Solitario"));
    app.setApplicationVersion("0.1.0");
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("solitario"),
                        QIcon::fromTheme(QStringLiteral("applications-games"))));

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
