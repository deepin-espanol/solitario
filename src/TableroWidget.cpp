#include "TableroWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontInfo>
#include <QLinearGradient>

void TableroWidget::IniciarJuego() {
    dragging = false;
    dragCartas.clear();
    dragCol = -1;
    dragIdx = -1;
    dropColResaltado = -1;

    klondike.Iniciar();
    update();
}

int TableroWidget::AnchoCarta() const {
    return qMax(40, qMin(70, (width() - 40) / 7));
}

int TableroWidget::AltoCarta() const {
    return int(AnchoCarta() * 1.45);
}

int TableroWidget::EspaciadoX() const {
    return qMax(4, (width() - 7 * AnchoCarta()) / (7 + 1));
}

int TableroWidget::EspaciadoY() const {
    return AltoCarta() / 4;
}

QRect TableroWidget::ColumnaRect(int col) const {
    int w = AnchoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = topY + AltoCarta() + sy * 2;
    int x = sx + col * (w + sx);
    return QRect(x, colY, w, height() - colY);
}

int TableroWidget::ColumnaObjetivo(int mx, int my) const {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = topY + h + sy * 2;

    for (int i = 0; i < 7; i++) {
        int x = sx + i * (w + sx);
        QRect zone(x - sx/2, colY - sy, w + sx, height() - colY + sy);
        if (zone.contains(mx, my)) return i;
    }
    return -1;
}

TableroWidget::TableroWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(800, 600);
    setMouseTracking(true);
    setAutoFillBackground(false);
}

void TableroWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    DibujarFondo(p);
    DibujarKlondike(p);
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

void TableroWidget::DibujarCarta(QPainter &p, const Carta &c, int x, int y, int w, int h) {
    QRect r(x, y, w, h);

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
    int ty = y + pad;
    p.setFont(fv);
    p.drawText(tx, ty + mfv.ascent(), valor);
    p.setFont(fs);
    p.drawText(tx, ty + mfv.height() + mfs.ascent() + 1, palo);

    p.save();
    p.translate(x + w - pad, y + h - pad);
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

void TableroWidget::DibujarCartaBocaAbajo(QPainter &p, int x, int y, int w, int h) {
    QRect r(x, y, w, h);

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
            int px = dragMouseActual.x() - w/2;
            int py = dragMouseActual.y() + i * EspaciadoY();
            DibujarCarta(p, dragCartas[i], px, py, w, h);
        }
    }
}

int TableroWidget::HitTest(int mx, int my, int &col, int &idx) {
    int w = AnchoCarta(), h = AltoCarta();
    int sx = EspaciadoX(), sy = EspaciadoY();
    int topY = sy;
    int colY = topY + h + sy * 2;

    for (int i = 0; i < 4; i++) {
        int fx = width() - (4 - i) * (w + sx) + sx;
        if (QRect(fx, topY, w, h).contains(mx, my)) {
            col = -2 - i;
            idx = -1;
            return 0;
        }
    }

    if (QRect(sx, topY, w, h).contains(mx, my)) {
        col = -1; idx = -1; return 1;
    }
    int wasteX = sx + w + sx;
    if (!klondike.Waste().isEmpty() && QRect(wasteX, topY, w, h).contains(mx, my)) {
        col = -1;
        idx = klondike.WasteMonton().Tamanio() - 1;
        return 2;
    }

    for (int i = 6; i >= 0; i--) {
        int x = sx + i * (w + sx);
        Monton &m = klondike.Columnas()[i];
        int numCartas = m.Tamanio();

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
        update();
        return;
    }

    if (hit >= 5) {
        IniciarDrag(event->pos().x(), event->pos().y());
    } else if (hit == 2) {
        dragCol = -1;
        dragIdx = 0;
        dragCartas.clear();
        dragCartas.append(klondike.WasteMonton().Top());
        dragging = true;
        dragMouseActual = event->pos();
    }
    update();
}

void TableroWidget::IniciarDrag(int mx, int my) {
    int col, idx;
    HitTest(mx, my, col, idx);

    if (col < 0) return;

    QVector<Carta> &cartas = klondike.Columnas()[col].cartas;

    if (idx >= cartas.size()) return;
    if (!cartas[idx].bocaArriba) {
        if (idx == cartas.size() - 1) {
            cartas[idx].bocaArriba = true;
            update();
        }
        return;
    }

    dragCol = col;
    dragIdx = idx;
    dragCartas.clear();
    for (int i = idx; i < cartas.size(); i++) {
        dragCartas.append(cartas[i]);
    }
    dragging = true;
    dragMouseActual = QPoint(mx, my);
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

    if (dragCol == -1) {
        if (col >= 0) moved = klondike.MoverAColumna(-1, 0, col);
        if (!moved) moved = klondike.MoverAFoundation(-1, 0);
    } else if (col >= 0) {
        moved = klondike.MoverAColumna(dragCol, dragIdx, col);
    } else if (col <= -2 && col >= -5) {
        if (dragCol == -1) moved = klondike.MoverAFoundation(-1, 0);
        else moved = klondike.MoverAFoundation(dragCol, dragIdx);
    }

    if (moved) {
        dragCartas.clear();
        dropColResaltado = -1;
        klondike.autoFoundation();
        update();
    } else {
        update();
    }
}

bool TableroWidget::ValidarDropEnColumna(int tgtCol) {
    if (tgtCol < 0 || dragCartas.isEmpty()) return false;

    const Carta &first = dragCartas.first();
    const Monton &dest = klondike.Columnas()[tgtCol];
    if (dest.Vacia()) return first.valor == 13;
    const Carta &top = dest.Top();
    return top.EsRojo() != first.EsRojo() && top.valor == first.valor + 1;
}

void TableroWidget::resizeEvent(QResizeEvent *) {
    update();
}
