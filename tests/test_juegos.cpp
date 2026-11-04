// Tests de la logica de las tres variantes (sin interfaz grafica).
#include <QtTest>
#include <QRandomGenerator>
#include <QJsonDocument>

#include "JuegoKlondike.h"
#include "JuegoSpider.h"
#include "JuegoFreeCell.h"

namespace {

Carta C(int valor, Palo palo, bool arriba = true) { return Carta{palo, valor, arriba}; }

Monton M(std::initializer_list<Carta> cartas)
{
    Monton m;
    for (const Carta &c : cartas) m.Agregar(c);
    return m;
}

QByteArray huella(const EstadoMesa &e)
{
    return QJsonDocument(SerialMesa::estadoToJson(e)).toJson(QJsonDocument::Compact);
}

} // namespace

class TestJuegos : public QObject {
    Q_OBJECT

private slots:
    // ---------------- Klondike ----------------

    // Deshacer debe volver a tapar la carta que se destapo al mover
    void klondike_deshacerVuelveATapar()
    {
        JuegoKlondike k;
        k.Iniciar();
        EstadoMesa e = k.Capturar();
        e.columnas[0] = M({C(9, Palo::Espada, false), C(8, Palo::Corazon)});
        e.columnas[1] = M({C(9, Palo::Trebol)});
        k.Restaurar(e);

        QVERIFY(k.MoverAColumna(0, 1, 1));               // 8♥ sobre 9♣
        QVERIFY(k.Columnas()[0].Top().bocaArriba);         // 9♠ destapado
        QVERIFY(k.Deshacer());
        QCOMPARE(k.Columnas()[0].Tamanio(), 2);
        QVERIFY(!k.Columnas()[0].cartas[0].bocaArriba);    // vuelve a estar tapado
        QVERIFY(k.Rehacer());
        QVERIFY(k.Columnas()[0].Top().bocaArriba);
    }

    void klondike_roboDeTres()
    {
        JuegoKlondike k;
        k.Iniciar(Dificultad::Dificil);
        QCOMPARE(k.CartasPorRobo(), 3);
        const int antes = k.Stock().size();
        QVERIFY(k.sacarDeStock());
        QCOMPARE(k.Waste().size(), 3);
        QCOMPARE(k.Stock().size(), antes - 3);
        QVERIFY(k.Deshacer());
        QCOMPARE(k.Waste().size(), 0);
        QCOMPARE(k.Stock().size(), antes);
    }

    // No sube a la fundacion un 4 negro si los 3 rojos aun hacen falta
    void klondike_autoFundacionSoloSegura()
    {
        JuegoKlondike k;
        k.Iniciar();
        EstadoMesa e = k.Capturar();
        for (auto &c : e.columnas) c.Limpiar();
        e.stock.Limpiar();
        e.waste.Limpiar();
        e.foundations[0] = M({C(1, Palo::Trebol), C(2, Palo::Trebol), C(3, Palo::Trebol)});
        e.foundations[1] = M({C(1, Palo::Corazon)});
        e.columnas[0] = M({C(4, Palo::Trebol)});
        k.Restaurar(e);

        QVERIFY(!k.autoFoundation(true));
        QCOMPARE(k.Columnas()[0].Tamanio(), 1);
        QVERIFY(k.autoFoundation(false));                  // forzado: si sube
        QCOMPARE(k.Columnas()[0].Tamanio(), 0);
    }

    // ---------------- Spider ----------------

    // Completar una escalera y deshacer debe dejarlo todo como estaba
    void spider_deshacerEscaleraCompleta()
    {
        JuegoSpider s;
        s.Iniciar(Dificultad::Facil);
        EstadoMesa e = s.Capturar();
        for (auto &c : e.columnas) c.Limpiar();
        for (int v = 13; v >= 2; --v) e.columnas[0].Agregar(C(v, Palo::Trebol));
        e.columnas[1] = M({C(5, Palo::Corazon, false), C(1, Palo::Trebol)});
        s.Restaurar(e);
        const QByteArray inicial = huella(s.Capturar());

        QVERIFY(s.MoverAColumna(1, 1, 0));
        for (int i = 0; i < 10; i++) s.CompletarSecuencia(i);  // como hace el tablero
        QCOMPARE(s.Columnas()[0].Tamanio(), 0);
        QVERIFY(s.Columnas()[1].Top().bocaArriba);

        QVERIFY(s.Deshacer());
        QCOMPARE(huella(s.Capturar()), inicial);
    }

    // Deshacer no debe tapar una carta que ya estaba boca arriba
    void spider_deshacerNoTapaCartaVisible()
    {
        JuegoSpider s;
        s.Iniciar(Dificultad::Facil);
        EstadoMesa e = s.Capturar();
        for (auto &c : e.columnas) c.Limpiar();
        e.columnas[0] = M({C(9, Palo::Trebol), C(5, Palo::Trebol)});
        e.columnas[1] = M({C(6, Palo::Trebol)});
        s.Restaurar(e);

        QVERIFY(s.MoverAColumna(0, 1, 1));
        QVERIFY(s.Deshacer());
        QVERIFY(s.Columnas()[0].cartas[0].bocaArriba);
    }

    // ---------------- FreeCell ----------------

    // Movimientos a celdas libres y fundacion tambien se deshacen
    void freecell_deshacerTodoTipoDeMovimiento()
    {
        JuegoFreeCell f;
        f.Iniciar();
        const QByteArray inicial = huella(f.Capturar());
        const int col = 0;
        const int idx = f.Columnas()[col].Tamanio() - 1;
        QVERIFY(f.MoverACeldaLibre(col, idx));
        QVERIFY(f.MoverACeldaLibreDesdeCeldaLibre(0, 1));
        QVERIFY(f.Deshacer());
        QVERIFY(f.Deshacer());
        QCOMPARE(huella(f.Capturar()), inicial);
    }

    // Hacia una columna vacia, esa columna no cuenta como auxiliar
    void freecell_maxCartasAColumnaVacia()
    {
        JuegoFreeCell f;
        f.Iniciar();
        EstadoMesa e = f.Capturar();
        e.columnas[7].Limpiar();
        for (auto &c : e.celdas) c = Carta{};
        f.Restaurar(e);
        // 4 celdas libres, una sola columna vacia que es el destino
        QCOMPARE(f.MaxCartasMovibles(7, 0), 5);
        QCOMPARE(f.MaxCartasMovibles(1, 0), 10);
    }

    void freecell_autocompletaSoloSiEstaResuelta()
    {
        JuegoFreeCell f;
        f.Iniciar();
        EstadoMesa e = f.Capturar();
        for (auto &c : e.columnas) c.Limpiar();
        for (auto &c : e.celdas) c = Carta{};
        // Cada palo en su columna, de K a A: resoluble directamente
        for (int p = 0; p < 4; ++p)
            for (int v = 13; v >= 1; --v)
                e.columnas[p].Agregar(C(v, static_cast<Palo>(p)));
        f.Restaurar(e);
        QVERIFY(f.ResolubleAutomaticamente());
        f.autoFoundation(false);
        QVERIFY(f.HaGanado());

        f.Iniciar();  // reparto aleatorio: casi nunca resoluble de inicio
        EstadoMesa d = f.Capturar();
        d.columnas[0] = M({C(2, Palo::Trebol), C(5, Palo::Corazon)});
        f.Restaurar(d);
        QVERIFY(!f.ResolubleAutomaticamente());
    }

    // ---------------- Prueba aleatoria ----------------

    // Muchas jugadas legales al azar; deshacer todo debe volver al reparto
    // inicial y rehacer todo, al estado final.
    void aleatorio_deshacerYRehacerTodo()
    {
        auto *rng = QRandomGenerator::global();
        for (int partida = 0; partida < 30; ++partida) {
            const int variante = partida % 3;
            JuegoKlondike k;
            JuegoSpider s;
            JuegoFreeCell f;
            if (variante == 0) k.Iniciar(partida % 2 ? Dificultad::Dificil : Dificultad::Medio);
            if (variante == 1) s.Iniciar(Dificultad::Medio);
            if (variante == 2) f.Iniciar(Dificultad::Medio);
            auto captura = [&]() {
                return variante == 0 ? k.Capturar() : variante == 1 ? s.Capturar() : f.Capturar();
            };
            const QByteArray inicial = huella(captura());

            int hechas = 0;
            for (int intento = 0; intento < 4000 && hechas < 150; ++intento) {
                bool ok = false;
                const int a = rng->bounded(10), b = rng->bounded(10);
                // Cartas cerca de la cima: indices al azar casi nunca son jugadas legales
                auto cerca = [&](const Monton &m) { return m.Tamanio() - 1 - int(rng->bounded(4)); };
                switch (variante) {
                case 0:
                    switch (rng->bounded(4)) {
                    case 0: ok = k.sacarDeStock(); break;
                    case 1: ok = (a % 8 == 0) ? k.MoverAColumna(-1, 0, b % 7)
                                              : k.MoverAColumna(a % 8 - 1, cerca(k.Columnas()[a % 8 - 1]), b % 7); break;
                    case 2: ok = k.MoverAFoundation(-1, 0); break;
                    case 3: ok = k.MoverAFoundation(a % 7, k.Columnas()[a % 7].Tamanio() - 1); break;
                    }
                    if (ok) k.autoFoundation(true);
                    break;
                case 1:
                    ok = (rng->bounded(25) == 0) ? s.sacarDeStock() : s.MoverAColumna(a, cerca(s.Columnas()[a]), b);
                    if (ok) for (int c = 0; c < 10; c++) s.CompletarSecuencia(c);
                    break;
                case 2:
                    switch (rng->bounded(4)) {
                    case 0: ok = f.MoverAColumna(a % 8, cerca(f.Columnas()[a % 8]), b % 8); break;
                    case 1: ok = f.MoverACeldaLibre(a % 8, f.Columnas()[a % 8].Tamanio() - 1); break;
                    case 2: ok = f.MoverAColumnaDesdeCeldaLibre(a % 4, b % 8); break;
                    case 3: ok = f.MoverAFoundation(a % 8, f.Columnas()[a % 8].Tamanio() - 1); break;
                    }
                    if (ok) f.autoFoundation(true);
                    break;
                }
                if (ok) ++hechas;
            }
            QVERIFY2(hechas > 0, qPrintable(QStringLiteral("variante %1: solo %2 jugadas").arg(variante).arg(hechas)));
            const QByteArray final_ = huella(captura());

            auto deshacer = [&]() { return variante == 0 ? k.Deshacer() : variante == 1 ? s.Deshacer() : f.Deshacer(); };
            auto rehacer = [&]() { return variante == 0 ? k.Rehacer() : variante == 1 ? s.Rehacer() : f.Rehacer(); };
            int n = 0;
            while (deshacer()) ++n;
            QCOMPARE(n, hechas);
            QCOMPARE(huella(captura()), inicial);
            while (rehacer()) {}
            QCOMPARE(huella(captura()), final_);
        }
    }
};

QTEST_GUILESS_MAIN(TestJuegos)
#include "test_juegos.moc"
