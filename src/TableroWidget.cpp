#include "TableroWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontInfo>
#include <QLinearGradient>
#include <QSoundEffect>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QKeySequence>
#include <cmath>

static QByteArray GenerarTono(int freq, int durationMs, int sampleRate = 44100) {
    int numSamples = sampleRate * durationMs / 1000;
    int dataSize = numSamples * 2;
    int fileSize = 44 + dataSize;

    QByteArray buf(fileSize, 0);
    auto put32 = [&](int off, int v) { buf[off]=v; buf[off+1]=v>>8; buf[off+2]=v>>16; buf[off+3]=v>>24; };
    auto put16 = [&](int off, int v) { buf[off]=v; buf[off+1]=v>>8; };

    buf.replace(0, 4, "RIFF");
    put32(4, fileSize - 8);
    buf.replace(8, 4, "WAVE");
    buf.replace(12, 4, "fmt ");
    put32(16, 16);
    put16(20, 1);
    put16(22, 1);
    put32(24, sampleRate);
    put32(28, sampleRate * 2);
    put16(32, 2);
    put16(34, 16);
    buf.replace(36, 4, "data");
    put32(40, dataSize);

    qint16 *samples = reinterpret_cast<qint16*>(buf.data() + 44);
    for (int i = 0; i < numSamples; i++) {
        double t = (double)i / sampleRate;
        double env = 1.0 - (double)i / numSamples;
        samples[i] = (qint16)(16000.0 * env * sin(2.0 * M_PI * freq * t));
    }
    return buf;
}

static QString SonidosPath() {
    static QString path;
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(path);
    }
    return path;
}

void TableroWidget::IniciarJuego(Juego tipo) {
    tipoJuego = tipo;
    dragging = false;
    dragCartas.clear();
    dragCol = -1;
    dragIdx = -1;
    dropColResaltado = -1;
    movimientos = 0;
    tiempoSegundos = 0;
    puntuacion = 0;
    animTimer.stop();
    timerReloj.stop();

    switch (tipo) {
    case Juego::Klondike: klondike.Iniciar(dificultad); break;
    case Juego::Spider:   spider.Iniciar(dificultad);   break;
    case Juego::FreeCell: freecell.Iniciar(dificultad);  break;
    }

    // Shuffle animation
    shuffleCards.clear();
    barajando = true;
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int centerX = width() / 2 - w / 2;
    int centerY = height() / 2 - h / 2;

    auto addCards = [&](QVector<Monton> &cols, int numCols) {
        for (int i = 0; i < numCols; i++) {
            for (int j = 0; j < cols[i].Tamanio(); j++) {
                int toX = sx + i * (w + sx);
                int toY = (tipoJuego == Juego::Spider) ? (sx + h + sy * 2 + j * sy) : (sy + h + sy * 2 + j * sy);
                ShuffleCard sc;
                sc.fromX = centerX + (rand() % 40 - 20);
                sc.fromY = centerY + (rand() % 40 - 20);
                sc.toX = toX;
                sc.toY = toY;
                sc.progress = 0.0f;
                sc.carta = cols[i].cartas[j];
                sc.col = i;
                sc.idx = j;
                shuffleCards.append(sc);
            }
        }
    };

    switch (tipo) {
    case Juego::Klondike: addCards(klondike.Columnas(), 7); break;
    case Juego::Spider:   addCards(spider.Columnas(), 10);  break;
    case Juego::FreeCell: addCards(freecell.Columnas(), 8);  break;
    }

    if (!shuffleCards.isEmpty()) {
        float step = 0.04f;
        for (int i = 0; i < shuffleCards.size(); i++) {
            shuffleCards[i].progress = qMin(1.0f, step * i);
        }
        shuffleTimer.start();
    } else {
        barajando = false;
        timerReloj.start();
    }

    emit MovimientosChanged(0);
    emit TiempoChanged(0);
    emit PuntuacionChanged(0);
    update();
}

void TableroWidget::AnimarBarajado() {
    bool allDone = true;
    for (auto &sc : shuffleCards) {
        if (sc.progress < 1.0f) {
            sc.progress = qMin(1.0f, sc.progress + 0.06f);
            allDone = false;
        }
    }
    update();
    if (allDone) {
        shuffleTimer.stop();
        barajando = false;
        shuffleCards.clear();
        timerReloj.start();
    }
}

void TableroWidget::TickTemporizador() {
    if (barajando) return;
    tiempoSegundos++;
    emit TiempoChanged(tiempoSegundos);
}

void TableroWidget::CalcularPuntuacion() {
    int base = 0;
    switch (tipoJuego) {
    case Juego::Klondike:
        for (int i = 0; i < 4; i++) base += klondike.Foundations()[i].Tamanio() * 10;
        base -= movimientos;
        break;
    case Juego::Spider:
        for (int i = 0; i < 8; i++) base += spider.Foundations()[i].Tamanio() * 100;
        base -= movimientos;
        break;
    case Juego::FreeCell:
        for (int i = 0; i < 4; i++) base += freecell.Foundations()[i].Tamanio() * 10;
        base -= movimientos;
        break;
    }
    int bonus = qMax(0, 600 - tiempoSegundos);
    puntuacion = qMax(0, base + bonus);
    emit PuntuacionChanged(puntuacion);
}

TableroWidget::TableroWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(800, 600);
    setMouseTracking(true);
    setAutoFillBackground(false);

    animTimer.setTimerType(Qt::PreciseTimer);
    animTimer.setInterval(16);
    connect(&animTimer, &QTimer::timeout, this, &TableroWidget::AnimarDrag);

    timerReloj.setTimerType(Qt::CoarseTimer);
    timerReloj.setInterval(1000);
    connect(&timerReloj, &QTimer::timeout, this, &TableroWidget::TickTemporizador);

    shuffleTimer.setTimerType(Qt::PreciseTimer);
    shuffleTimer.setInterval(16);
    connect(&shuffleTimer, &QTimer::timeout, this, &TableroWidget::AnimarBarajado);

    QString sPath = SonidosPath();
    QByteArray flipData = GenerarTono(1200, 60);
    QByteArray moveData = GenerarTono(600, 100);
    QByteArray winData  = GenerarTono(800, 300);

    QString flipFile = sPath + "/flip.wav";
    QString moveFile = sPath + "/move.wav";
    QString winFile  = sPath + "/win.wav";
    QFile(flipFile).remove();
    QFile(moveFile).remove();
    QFile(winFile).remove();

    QFile f1(flipFile); f1.open(QIODevice::WriteOnly); f1.write(flipData); f1.close();
    QFile f2(moveFile); f2.open(QIODevice::WriteOnly); f2.write(moveData); f2.close();
    QFile f3(winFile);  f3.open(QIODevice::WriteOnly); f3.write(winData);  f3.close();

    sndFlip = new QSoundEffect(this);
    sndFlip->setSource(QUrl::fromLocalFile(flipFile));
    sndFlip->setVolume(0.5f);

    sndMove = new QSoundEffect(this);
    sndMove->setSource(QUrl::fromLocalFile(moveFile));
    sndMove->setVolume(0.4f);

    sndWin = new QSoundEffect(this);
    sndWin->setSource(QUrl::fromLocalFile(winFile));
    sndWin->setVolume(0.6f);
}

void TableroWidget::ReproducirSonido(const QString &tipo) {
    QSoundEffect *s = nullptr;
    if (tipo == "flip") s = sndFlip;
    else if (tipo == "move") s = sndMove;
    else if (tipo == "win") s = sndWin;
    if (s && !s->isLoaded()) return;
    if (s) s->play();
}

int TableroWidget::AnchoCarta() const {
    int cols = (tipoJuego == Juego::Spider) ? 10 :
               (tipoJuego == Juego::FreeCell) ? 8 : 7;
    return qMax(40, qMin(70, (width() - 40) / cols));
}

int TableroWidget::AltoCarta() const {
    return int(AnchoCarta() * 1.45);
}

int TableroWidget::EspaciadoX() const {
    int cols = (tipoJuego == Juego::Spider) ? 10 :
               (tipoJuego == Juego::FreeCell) ? 8 : 7;
    return qMax(4, (width() - cols * AnchoCarta()) / (cols + 1));
}

int TableroWidget::EspaciadoY() const {
    return AltoCarta() / 4;
}

QRect TableroWidget::ColumnaRect(int col) const {
    int w = AnchoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = (tipoJuego == Juego::Spider) ? (sx + AltoCarta() + sy * 2) : (topY + AltoCarta() + sy * 2);
    int x = sx + col * (w + sx);
    return QRect(x, colY, w, height() - colY);
}

int TableroWidget::ColumnaObjetivo(int mx, int my) const {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = (tipoJuego == Juego::Spider) ? (sx + h + sy * 2) : (topY + h + sy * 2);
    int numCols = (tipoJuego == Juego::Spider) ? 10 :
                  (tipoJuego == Juego::FreeCell) ? 8 : 7;

    for (int i = 0; i < numCols; i++) {
        int x = sx + i * (w + sx);
        QRect zone(x - sx/2, colY - sy, w + sx, height() - colY + sy);
        if (zone.contains(mx, my)) return i;
    }
    return -1;
}

void TableroWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    DibujarFondo(p);

    if (barajando) {
        int w = AnchoCarta(), h = AltoCarta();
        for (auto &sc : shuffleCards) {
            float t = 1.0f - std::pow(1.0f - sc.progress, 3);
            int x = sc.fromX + (int)((sc.toX - sc.fromX) * t);
            int y = sc.fromY + (int)((sc.toY - sc.fromY) * t);
            DibujarCartaBocaAbajo(p, x, y, w, h, (1.0f - t) * 0.3f);
        }
        return;
    }

    switch (tipoJuego) {
    case Juego::Klondike: DibujarKlondike(p); break;
    case Juego::Spider:   DibujarSpider(p);   break;
    case Juego::FreeCell: DibujarFreeCell(p); break;
    }
}

void TableroWidget::DibujarFondo(QPainter &p) {
    QColor top, bottom, border;
    switch (fondoVerde) {
    case 0:
        top = QColor(0, 100, 45);
        bottom = QColor(0, 70, 30);
        border = QColor(0, 60, 25);
        break;
    case 1:
        top = QColor(18, 32, 20);
        bottom = QColor(8, 18, 10);
        border = QColor(5, 12, 8);
        break;
    case 2:
        top = QColor(50, 170, 90);
        bottom = QColor(35, 130, 65);
        border = QColor(25, 100, 50);
        break;
    default:
        top = QColor(0, 100, 45);
        bottom = QColor(0, 70, 30);
        border = QColor(0, 60, 25);
    }

    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0, top);
    grad.setColorAt(1, bottom);
    p.fillRect(rect(), grad);

    p.setPen(QPen(border, 2));
    p.drawRect(rect().adjusted(1, 1, -2, -2));

    p.setPen(QPen(QColor(255, 255, 255, 8), 1));
    int step = 12;
    for (int y = 0; y < height(); y += step) {
        p.drawLine(0, y, width(), y);
    }
    for (int x = 0; x < width(); x += step) {
        p.drawLine(x, 0, x, height());
    }
}

void TableroWidget::DibujarCarta(QPainter &p, const Carta &c, int x, int y, int w, int h, float lift) {
    QRect r(x, y, w, h);

    int blur = int(lift * 14);
    if (blur > 0) DibujarSombra(p, x, y, w, h, blur, QColor(0, 0, 0, int(120 * lift)));

    p.setPen(QPen(QColor(150, 150, 150), 1));
    p.setBrush(QColor(248, 248, 240));
    p.drawRoundedRect(r, 4, 4);

    p.setPen(QPen(QColor(200, 200, 195), 0.5));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r.adjusted(3, 3, -3, -3), 3, 3);

    QColor color = c.EsRojo() ? QColor(185, 25, 25) : QColor(25, 25, 25);
    QString palo = c.PaloStr();
    QString valor = c.ValorStr();

    int valPx = qMax(8, qMin(w / 3, 18));
    int suitPx = qMax(6, qMin(w / 6, 11));

    QFont fv("Noto Sans CJK SC");
    fv.setPixelSize(valPx);
    fv.setBold(true);
    if (!QFontInfo(fv).exactMatch()) {
        fv = QFont("Noto Sans");
        fv.setPixelSize(valPx);
        fv.setBold(true);
    }

    QFont fs("Noto Sans CJK SC");
    fs.setPixelSize(suitPx);
    fs.setBold(true);
    if (!QFontInfo(fs).exactMatch()) {
        fs = QFont("Noto Sans");
        fs.setPixelSize(suitPx);
        fs.setBold(true);
    }

    QFontMetrics mfv(fv);
    QFontMetrics mfs(fs);
    int pad = qMax(2, w / 16);

    p.save();
    p.setPen(color);

    int tx = x + pad;
    int ty = y + pad - int(lift * 4);
    p.setFont(fv);
    p.drawText(tx, ty + mfv.ascent(), valor);
    p.setFont(fs);
    p.drawText(tx, ty + mfv.height() + mfs.ascent() + 1, palo);

    p.save();
    p.translate(x + w - pad, y + h - pad + int(lift * 4));
    p.rotate(180);
    p.setFont(fv);
    p.drawText(0, mfv.ascent(), valor);
    p.setFont(fs);
    p.drawText(0, mfv.height() + mfs.ascent() + 1, palo);
    p.restore();

    int centerPx = qMax(10, qMin(w / 2, 24));
    QFont cs("Noto Sans CJK SC");
    cs.setPixelSize(centerPx);
    if (!QFontInfo(cs).exactMatch()) {
        cs = QFont("Noto Sans");
        cs.setPixelSize(centerPx);
    }
    p.setFont(cs);
    p.setOpacity(0.10);
    p.drawText(r, Qt::AlignCenter, palo);
    p.setOpacity(1.0);

    p.restore();
}

void TableroWidget::DibujarSombra(QPainter &p, int x, int y, int w, int h, int blur, QColor color) {
    if (blur <= 0 || color.alpha() <= 0) return;
    QRect r(x - blur/3, y + blur/2, w + blur/3, h + blur/2);
    int steps = blur;
    int alphaStep = color.alpha() / qMax(1, steps);
    p.save();
    p.setPen(Qt::NoPen);
    for (int i = steps; i > 0; i -= 2) {
        p.setBrush(QColor(color.red(), color.green(), color.blue(), color.alpha() - i * alphaStep * 3));
        p.drawRoundedRect(r.adjusted(-i/2, -i/2, i/2, i/2 + i), 4 + i, 4 + i);
    }
    p.restore();
}

void TableroWidget::DibujarCartaBocaAbajo(QPainter &p, int x, int y, int w, int h, float lift) {
    QRect r(x, y, w, h);

    int blur = int(lift * 14);
    if (blur > 0) DibujarSombra(p, x, y, w, h, blur, QColor(0, 0, 0, int(120 * lift)));

    QColor rojoBase(213, 51, 51);
    QColor dorado(218, 165, 32);
    QColor doradoClaro(240, 200, 80, 120);

    p.setPen(QPen(dorado, 1.5));
    p.setBrush(rojoBase);
    p.drawRoundedRect(r, 4, 4);

    p.setPen(QPen(doradoClaro, 0.8));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r.adjusted(3, 3, -3, -3), 3, 3);

    p.save();
    p.setClipRegion(QRegion(r.adjusted(5, 5, -5, -5)));

    int step = qMax(6, w / 5);
    p.setPen(QPen(doradoClaro, 0.5));
    for (int sx2 = r.left() + 6; sx2 < r.right() - 2; sx2 += step) {
        for (int sy2 = r.top() + 6; sy2 < r.bottom() - 2; sy2 += step) {
            int hs = step / 6;
            QPointF diamond[] = {
                QPointF(sx2, sy2 - hs),
                QPointF(sx2 + hs, sy2),
                QPointF(sx2, sy2 + hs),
                QPointF(sx2 - hs, sy2)
            };
            p.drawPolygon(diamond, 4);
        }
    }

    int cx = x + w/2;
    int cy = y + h/2;
    int medSize = qMin(w, h) / 3;

    p.setPen(QPen(dorado, 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(cx, cy), medSize, medSize);

    p.setPen(QPen(doradoClaro, 1.2));
    p.drawEllipse(QPointF(cx, cy), medSize * 0.70, medSize * 0.70);

    p.drawLine(cx - medSize, cy, cx + medSize, cy);
    p.drawLine(cx, cy - medSize, cx, cy + medSize);

    p.setPen(QPen(dorado, 2.5));
    p.setBrush(dorado);
    p.drawEllipse(QPointF(cx, cy), 3, 3);

    int cornerPad = qMax(4, w / 10);
    int cornerSize = qMax(4, w / 12);
    QPointF corners[] = {
        {qreal(x + cornerPad), qreal(y + cornerPad)},
        {qreal(x + w - cornerPad - cornerSize), qreal(y + cornerPad)},
        {qreal(x + cornerPad), qreal(y + h - cornerPad - cornerSize)},
        {qreal(x + w - cornerPad - cornerSize), qreal(y + h - cornerPad - cornerSize)}
    };
    for (auto &cp : corners) {
        p.setPen(QPen(dorado, 1.2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(cp.x(), cp.y(), cornerSize, cornerSize), 2, 2);
        p.setBrush(dorado);
        p.drawEllipse(QPointF(cp.x() + cornerSize/2, cp.y() + cornerSize/2), 1.5, 1.5);
    }

    p.restore();
}

void TableroWidget::DibujarStockCount(QPainter &p, int count, int x, int y, int w, int h) {
    if (count <= 0) return;
    QFont f("Noto Sans");
    f.setPixelSize(qMax(9, qMin(12, w / 4)));
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor(255, 255, 255, 200));
    QRect badge(x + w - 18, y + h - 16, 18, 16);
    p.setBrush(QColor(0, 0, 0, 120));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(badge, 4, 4);
    p.setPen(QColor(255, 255, 255, 220));
    p.drawText(badge, Qt::AlignCenter, QString::number(count));
}

void TableroWidget::DibujarDropTarget(QPainter &p, int col) {
    if (col < 0) return;
    int w = AnchoCarta();
    QRect zone = ColumnaRect(col);
    int x = zone.x();
    int y = zone.y();
    int h = zone.height();
    QColor cBase = dropValido ? QColor(255, 255, 100) : QColor(255, 80, 80);
    QColor fill = dropValido ? QColor(255, 255, 100, 30) : QColor(255, 80, 80, 25);

    p.setPen(QPen(cBase, 2, Qt::DashLine));
    p.setBrush(fill);
    p.drawRoundedRect(QRect(x, y, w, h), 6, 6);
}

void TableroWidget::DibujarKlondike(QPainter &p) {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;

    if (dragging) DibujarDropTarget(p, dropColResaltado);

    for (int i = 0; i < 4; i++) {
        int x = width() - (4 - i) * (w + sx) + sx;
        QRect r(x, topY, w, h);
        p.setPen(QPen(QColor(255, 255, 255, 60), 1, Qt::DashLine));
        p.setBrush(QColor(255, 255, 255, 15));
        p.drawRoundedRect(r, 4, 4);
        if (!klondike.Foundations()[i].Vacia()) {
            DibujarCarta(p, klondike.Foundations()[i].Top(), x, topY, w, h);
        }
    }

    int stockX = sx;
    QRect stockR(stockX, topY, w, h);
    if (!klondike.Stock().isEmpty()) {
        DibujarCartaBocaAbajo(p, stockX, topY, w, h);
        DibujarStockCount(p, klondike.Stock().size(), stockX, topY, w, h);
    } else {
        p.setPen(QPen(QColor(255, 255, 255, 60), 1, Qt::DashLine));
        p.setBrush(QColor(255, 255, 255, 15));
        p.drawRoundedRect(stockR, 4, 4);
    }

    int wasteX = stockX + w + sx;
    if (!klondike.Waste().isEmpty()) {
        DibujarCarta(p, klondike.WasteMonton().Top(), wasteX, topY, w, h);
    }

    int colY = topY + h + sy * 2;
    for (int i = 0; i < 7; i++) {
        int x = sx + i * (w + sx);
        if (klondike.Columnas()[i].Vacia()) {
            QRect r(x, colY, w, h);
            p.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DashLine));
            p.setBrush(QColor(255, 255, 255, 10));
            p.drawRoundedRect(r, 4, 4);
        }
        for (int j = 0; j < klondike.Columnas()[i].Tamanio(); j++) {
            const Carta &c = klondike.Columnas()[i].cartas[j];
            int cy = colY + j * sy;
            if (dragging && i == dragCol && j >= dragIdx) continue;
            if (c.bocaArriba) DibujarCarta(p, c, x, cy, w, h);
            else DibujarCartaBocaAbajo(p, x, cy, w, h);
        }
    }

    if (dragging) {
        for (int i = 0; i < dragCartas.size(); i++) {
            int px = dragMouseFinal.x() - w/2;
            int py = dragMouseFinal.y() + i * EspaciadoY();
            DibujarCarta(p, dragCartas[i], px, py, w, h, dragLift);
        }
    }
}

void TableroWidget::DibujarSpider(QPainter &p) {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();

    if (dragging) DibujarDropTarget(p, dropColResaltado);

    if (!spider.StockMonton().Vacia()) {
        int remaining = spider.StockMonton().Tamanio() / 10;
        for (int i = 0; i < remaining && i < 5; i++) {
            DibujarCartaBocaAbajo(p, sx + i * 3, sx + i * 3, w, h);
        }
        if (remaining > 0) {
            DibujarStockCount(p, spider.StockMonton().Tamanio(), sx, sx, w, h);
        }
    }

    for (int i = 0; i < 8; i++) {
        if (!spider.Foundations()[i].Vacia()) {
            int x = width() - (8 - i) * (w / 2 + 4);
            DibujarCarta(p, spider.Foundations()[i].Top(), x, sx, w / 2, h * 2 / 3);
        }
    }

    int colY = sx + h + sy * 2;
    for (int i = 0; i < 10; i++) {
        int x = sx + i * (w + sx);
        if (spider.Columnas()[i].Vacia()) {
            QRect r(x, colY, w, h);
            p.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DashLine));
            p.setBrush(QColor(255, 255, 255, 10));
            p.drawRoundedRect(r, 4, 4);
        }
        for (int j = 0; j < spider.Columnas()[i].Tamanio(); j++) {
            const Carta &c = spider.Columnas()[i].cartas[j];
            int cy = colY + j * sy;
            if (dragging && i == dragCol && j >= dragIdx) continue;
            if (c.bocaArriba) DibujarCarta(p, c, x, cy, w, h);
            else DibujarCartaBocaAbajo(p, x, cy, w, h);
        }
    }

    if (dragging) {
        for (int i = 0; i < dragCartas.size(); i++) {
            int px = dragMouseFinal.x() - w/2;
            int py = dragMouseFinal.y() + i * EspaciadoY();
            DibujarCarta(p, dragCartas[i], px, py, w, h, dragLift);
        }
    }
}

void TableroWidget::DibujarFreeCell(QPainter &p) {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;

    if (dragging) DibujarDropTarget(p, dropColResaltado);

    int numFreeCells = freecell.CeldasLibres().size();
    for (int i = 0; i < numFreeCells; i++) {
        int x = sx + i * (w + sx);
        QRect r(x, topY, w, h);
        p.setPen(QPen(QColor(255, 255, 255, 60), 1, Qt::DashLine));
        p.setBrush(QColor(255, 255, 255, 15));
        p.drawRoundedRect(r, 4, 4);
        if (!freecell.CeldasLibres()[i].EsVacia()) {
            DibujarCarta(p, freecell.CeldasLibres()[i], x, topY, w, h);
        }
    }

    for (int i = 0; i < 4; i++) {
        int x = width() - (4 - i) * (w + sx) + sx;
        QRect r(x, topY, w, h);
        p.setPen(QPen(QColor(255, 255, 255, 60), 1, Qt::DashLine));
        p.setBrush(QColor(255, 255, 255, 15));
        p.drawRoundedRect(r, 4, 4);
        if (!freecell.Foundations()[i].Vacia()) {
            DibujarCarta(p, freecell.Foundations()[i].Top(), x, topY, w, h);
        }
    }

    int colY = topY + h + sy * 2;
    for (int i = 0; i < 8; i++) {
        int x = sx + i * (w + sx);
        if (freecell.Columnas()[i].Vacia()) {
            QRect r(x, colY, w, h);
            p.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DashLine));
            p.setBrush(QColor(255, 255, 255, 10));
            p.drawRoundedRect(r, 4, 4);
        }
        for (int j = 0; j < freecell.Columnas()[i].Tamanio(); j++) {
            const Carta &c = freecell.Columnas()[i].cartas[j];
            int cy = colY + j * sy;
            if (dragging && i == dragCol && j >= dragIdx) continue;
            DibujarCarta(p, c, x, cy, w, h);
        }
    }

    if (dragging) {
        for (int i = 0; i < dragCartas.size(); i++) {
            int px = dragMouseFinal.x() - w/2;
            int py = dragMouseFinal.y() + i * EspaciadoY();
            DibujarCarta(p, dragCartas[i], px, py, w, h, dragLift);
        }
    }
}

int TableroWidget::HitTest(int mx, int my, int &col, int &idx) {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = (tipoJuego == Juego::Spider) ? (sx + h + sy * 2) : (topY + h + sy * 2);

    if (tipoJuego != Juego::Spider) {
        for (int i = 0; i < 4; i++) {
            int fx = width() - (4 - i) * (w + sx) + sx;
            if (QRect(fx, topY, w, h).contains(mx, my)) {
                col = -2 - i;
                idx = -1;
                return 0;
            }
        }
    }

    if (tipoJuego == Juego::Klondike) {
        if (QRect(sx, topY, w, h).contains(mx, my)) {
            col = -1; idx = -1; return 1;
        }
        int wasteX = sx + w + sx;
        if (!klondike.Waste().isEmpty() && QRect(wasteX, topY, w, h).contains(mx, my)) {
            col = -1;
            idx = klondike.WasteMonton().Tamanio() - 1;
            return 2;
        }
    }

    if (tipoJuego == Juego::Spider) {
        if (!spider.StockMonton().Vacia() && QRect(sx, sx, w, h).contains(mx, my)) {
            col = -3; idx = -1; return 3;
        }
    }

    if (tipoJuego == Juego::FreeCell) {
        int numFree = freecell.CeldasLibres().size();
        for (int i = 0; i < numFree; i++) {
            int fx = sx + i * (w + sx);
            if (QRect(fx, topY, w, h).contains(mx, my)) {
                col = -4 - i; idx = -1; return 4;
            }
        }
    }

    int numCols = (tipoJuego == Juego::Spider) ? 10 :
                  (tipoJuego == Juego::FreeCell) ? 8 : 7;

    for (int i = numCols - 1; i >= 0; i--) {
        int x = sx + i * (w + sx);
        Monton *m = nullptr;
        switch (tipoJuego) {
        case Juego::Klondike: m = &klondike.Columnas()[i]; break;
        case Juego::Spider:   m = &spider.Columnas()[i];   break;
        case Juego::FreeCell: m = &freecell.Columnas()[i];  break;
        }
        int numCartas = m->Tamanio();

        for (int j = numCartas - 1; j >= 0; j--) {
            int cy = colY + j * sy;
            int ch = (j == numCartas - 1) ? h : sy;
            if (QRect(x, cy, w, ch).contains(mx, my) ||
                (j == numCartas - 1 && QRect(x, cy, w, h).contains(mx, my))) {
                col = i; idx = j; return 5 + i;
            }
        }
        if (numCartas == 0 && QRect(x, colY, w, h).contains(mx, my)) {
            col = i; idx = 0; return 5 + i;
        }
    }

    col = -1; idx = -1; return -1;
}

void TableroWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;

    int col, idx;
    int hit = HitTest(event->pos().x(), event->pos().y(), col, idx);

    if (hit == 1) {
        klondike.sacarDeStock();
        ReproducirSonido("flip");
        CalcularPuntuacion();
        update();
        return;
    }
    if (hit == 3) {
        if (spider.sacarDeStock()) {
            ReproducirSonido("flip");
            CalcularPuntuacion();
            update();
            return;
        }
    }

    if (hit >= 5) {
        IniciarDrag(event->pos().x(), event->pos().y());
    } else if (hit == 2) {
        dragCol = -1;
        dragIdx = 0;
        dragCartas.clear();
        dragCartas.append(klondike.WasteMonton().Top());
        dragging = true;
        dragLift = 0.0f;
        int w = AnchoCarta(), sx = EspaciadoX();
        int wasteX = sx + w + sx;
        dragOrigen = QPoint(wasteX + w/2, EspaciadoY());
        dragMouseActual = event->pos();
        dragMouseFinal = event->pos();
        animTimer.start();
    }
    update();
}

void TableroWidget::IniciarDrag(int mx, int my) {
    int col, idx;
    HitTest(mx, my, col, idx);

    if (tipoJuego == Juego::FreeCell && col <= -4 && col >= -7) {
        int celda = -(col + 4);
        if (celda < 0 || celda >= 4) return;
        if (freecell.CeldasLibres()[celda].EsVacia()) return;

        dragCol = col;
        dragIdx = -1;
        dragCartas.clear();
        dragCartas.append(freecell.CeldasLibres()[celda]);
        dragging = true;

        int w = AnchoCarta();
        int sx = EspaciadoX(), sy = EspaciadoY();
        int topY = sy;
        int x = sx + celda * (w + sx);
        dragOrigen = QPoint(x + w/2, topY);
        dragMouseActual = QPoint(mx, my);
        dragMouseFinal = dragOrigen;
        dragLift = 0.0f;
        animTimer.start();
        return;
    }

    if (col < 0) return;

    QVector<Carta> *cartas = nullptr;
    switch (tipoJuego) {
    case Juego::Klondike: cartas = &klondike.Columnas()[col].cartas; break;
    case Juego::Spider:   cartas = &spider.Columnas()[col].cartas;   break;
    case Juego::FreeCell: cartas = &freecell.Columnas()[col].cartas;  break;
    }

    if (idx >= cartas->size()) return;
    if (!(*cartas)[idx].bocaArriba) {
        if (idx == cartas->size() - 1) {
            (*cartas)[idx].bocaArriba = true;
            ReproducirSonido("flip");
            update();
        }
        return;
    }

    dragCol = col;
    dragIdx = idx;
    dragCartas.clear();
    for (int i = idx; i < cartas->size(); i++) {
        dragCartas.append((*cartas)[i]);
    }
    dragging = true;
    dragLift = 0.0f;

    int w = AnchoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = (tipoJuego == Juego::Spider) ? (sx + AltoCarta() + sy * 2) : (topY + AltoCarta() + sy * 2);
    int x = sx + col * (w + sx);
    int y = colY + idx * EspaciadoY();
    dragOrigen = QPoint(x + w/2, y);
    dragMouseActual = QPoint(mx, my);
    dragMouseFinal = dragOrigen;
    animTimer.start();
}
void TableroWidget::AnimarDrag() {
    bool needsUpdate = false;

    if (dragging && dragLift < 1.0f) {
        dragLift = qMin(1.0f, dragLift + 0.18f);
        needsUpdate = true;
    }

    if (dragging) {
        QPointF diff = dragMouseActual - dragMouseFinal;
        float k = 0.45f;
        QPoint delta(int(diff.x() * k), int(diff.y() * k));
        if (delta.manhattanLength() > 0) {
            dragMouseFinal += delta;
            needsUpdate = true;
        }
        if ((dragMouseActual - dragMouseFinal).manhattanLength() < 2) {
            dragMouseFinal = dragMouseActual;
        }
    }

    if (snapeando) {
        snapProgreso = qMin(1.0f, snapProgreso + 0.10f);
        float e = 1.0f - std::pow(1.0f - snapProgreso, 3);
        dragMouseFinal = snapOrigen + (snapDestino - snapOrigen) * e;
        dragLift = 1.0f - e;
        needsUpdate = true;
        if (snapProgreso >= 1.0f) {
            snapeando = false;
            dragCartas.clear();
            animTimer.stop();
        }
    }

    if (needsUpdate) update();
    else if (dragging && (dragMouseActual - dragMouseFinal).manhattanLength() == 0 && dragLift >= 1.0f) {
        animTimer.stop();
    }
}

void TableroWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!dragging) return;
    dragMouseActual = event->pos();

    int col, idx;
    HitTest(event->pos().x(), event->pos().y(), col, idx);
    int tgt = ColumnaObjetivo(event->pos().x(), event->pos().y());
    if (tgt >= 0 && tgt != dragCol) {
        dropColResaltado = tgt;
        dropValido = ValidarDropEnColumna(tgt);
    } else {
        dropColResaltado = -1;
    }
}

void TableroWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (!dragging) return;
    dragging = false;

    int col, idx;
    HitTest(event->pos().x(), event->pos().y(), col, idx);
    bool moved = false;

    if (tipoJuego == Juego::FreeCell && dragCol <= -4 && dragCol >= -7) {
        int celda = -(dragCol + 4);
        if (col >= 0) {
            moved = freecell.MoverAColumnaDesdeCeldaLibre(celda, col);
        } else if (col <= -2 && col >= -5) {
            moved = freecell.MoverAFoundationDesdeCeldaLibre(celda);
        } else if (col <= -4 && col >= -7 && col != dragCol) {
            int aCelda = -(col + 4);
            moved = freecell.MoverACeldaLibreDesdeCeldaLibre(celda, aCelda);
        }
    } else if (dragCol == -1 && tipoJuego == Juego::Klondike) {
        if (col >= 0) moved = klondike.MoverAColumna(-1, 0, col);
        if (!moved) moved = klondike.MoverAFoundation(-1, 0);
    } else if (col >= 0) {
        switch (tipoJuego) {
        case Juego::Klondike: moved = klondike.MoverAColumna(dragCol, dragIdx, col); break;
        case Juego::Spider:   moved = spider.MoverAColumna(dragCol, dragIdx, col); break;
        case Juego::FreeCell: moved = freecell.MoverAColumna(dragCol, dragIdx, col); break;
        }
    } else if (col <= -2 && col >= -5) {
        if (tipoJuego == Juego::Klondike) {
            if (dragCol == -1) moved = klondike.MoverAFoundation(-1, 0);
            else moved = klondike.MoverAFoundation(dragCol, dragIdx);
        } else if (tipoJuego == Juego::FreeCell) {
            if (dragCol >= 0) moved = freecell.MoverAFoundation(dragCol, dragIdx);
        }
    } else if (col <= -4 && col >= -7) {
        if (tipoJuego == Juego::FreeCell && dragCol >= 0 && dragCartas.size() == 1) {
            moved = freecell.MoverACeldaLibre(dragCol, dragIdx);
        }
    }

    if (moved) {
        ReproducirSonido("move");
        dragCartas.clear();
        dropColResaltado = -1;
        animTimer.stop();

        movimientos++;
        emit MovimientosChanged(movimientos);
        CalcularPuntuacion();

        if (tipoJuego == Juego::Spider) {
            for (int i = 0; i < 10; i++) spider.CompletarSecuencia(i);
        }
        if (tipoJuego == Juego::Klondike) klondike.autoFoundation();

        update();
        if (HaGanado()) {
            timerReloj.stop();
            CalcularPuntuacion();
            ReproducirSonido("win");
            emit Gano();
        }
    } else {
        snapOrigen = dragMouseFinal;
        snapDestino = dragOrigen;
        snapProgreso = 0.0f;
        snapeando = true;
        animTimer.start();
    }
}

void TableroWidget::Deshacer() {
    bool ok = false;
    switch (tipoJuego) {
    case Juego::Klondike: ok = klondike.Deshacer(); break;
    case Juego::Spider:   ok = spider.Deshacer();   break;
    case Juego::FreeCell: ok = freecell.Deshacer();  break;
    }
    if (ok) {
        movimientos++;
        emit MovimientosChanged(movimientos);
        CalcularPuntuacion();
        update();
    }
}

void TableroWidget::Rehacer() {
    bool ok = false;
    switch (tipoJuego) {
    case Juego::Klondike: ok = klondike.Rehacer(); break;
    case Juego::Spider:   ok = spider.Rehacer();   break;
    case Juego::FreeCell: ok = freecell.Rehacer();  break;
    }
    if (ok) {
        movimientos++;
        emit MovimientosChanged(movimientos);
        CalcularPuntuacion();
        update();
    }
}

void TableroWidget::CancelarDrag() {
    if (!dragging && !snapeando) return;
    dragging = false;
    snapeando = false;
    dropColResaltado = -1;
    dragCartas.clear();
    animTimer.stop();
    update();
}

void TableroWidget::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::Undo)) {
        Deshacer();
        return;
    }
    if (event->matches(QKeySequence::Redo)) {
        Rehacer();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        CancelarDrag();
        return;
    }
    QWidget::keyPressEvent(event);
}

bool TableroWidget::HaGanado() const {
    switch (tipoJuego) {
    case Juego::Klondike: return klondike.HaGanado();
    case Juego::Spider:   return spider.HaGanado();
    case Juego::FreeCell: return freecell.HaGanado();
    }
    return false;
}

bool TableroWidget::ValidarDropEnColumna(int tgtCol) {
    if (tgtCol < 0 || dragCartas.isEmpty()) return false;

    const Carta &first = dragCartas.first();

    switch (tipoJuego) {
    case Juego::Klondike: {
        const Monton &dest = klondike.Columnas()[tgtCol];
        if (dest.Vacia()) return first.valor == 13;
        const Carta &top = dest.Top();
        return top.EsRojo() != first.EsRojo() && top.valor == first.valor + 1;
    }
    case Juego::Spider: {
        const Monton &dest = spider.Columnas()[tgtCol];
        if (dest.Vacia()) return true;
        return dest.Top().valor == first.valor + 1;
    }
    case Juego::FreeCell: {
        const Monton &dest = freecell.Columnas()[tgtCol];
        if (dest.Vacia()) return true;
        const Carta &top = dest.Top();
        return top.EsRojo() != first.EsRojo() && top.valor == first.valor + 1;
    }
    }
    return false;
}

void TableroWidget::resizeEvent(QResizeEvent *) {
    update();
}
