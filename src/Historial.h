#ifndef HISTORIAL_H
#define HISTORIAL_H

#include "Carta.h"
#include "Monton.h"

#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

// Foto completa de la mesa. Deshacer/rehacer guarda una foto antes de cada
// accion y la restaura, en lugar de intentar invertir cada movimiento: asi
// los volteos automaticos, los movimientos a la fundacion encadenados y las
// escaleras completadas de Spider se deshacen siempre correctamente.
struct EstadoMesa {
    Monton stock;
    Monton waste;
    QVector<Monton> columnas;
    QVector<Monton> foundations;
    QVector<Carta> celdas;  // celdas libres (FreeCell)
};

// Copias profundas para las fotos del historial. QVector comparte el buffer al
// copiarse (copy-on-write): si la foto lo compartiera, escribir despues a
// traves de una referencia tomada antes de la foto (p. ej. `auto &dest =
// columnas[i]`) modificaria tambien la foto y deshacer no restauraria nada.
inline QVector<Carta> CopiaCartas(const QVector<Carta> &v)
{
    return QVector<Carta>(v.cbegin(), v.cend());
}

inline Monton CopiaMonton(const Monton &m)
{
    Monton r;
    r.cartas = CopiaCartas(m.cartas);
    return r;
}

inline QVector<Monton> CopiaMontones(const QVector<Monton> &v)
{
    QVector<Monton> r;
    r.reserve(v.size());
    for (const auto &m : v) r.append(CopiaMonton(m));
    return r;
}

namespace SerialMesa {

inline QJsonArray cartasToJson(const QVector<Carta> &cartas)
{
    QJsonArray arr;
    for (const auto &c : cartas) {
        QJsonObject obj;
        obj["palo"] = static_cast<int>(c.palo);
        obj["valor"] = c.valor;
        obj["boca"] = c.bocaArriba;
        arr.append(obj);
    }
    return arr;
}

inline QVector<Carta> cartasFromJson(const QJsonArray &arr)
{
    QVector<Carta> v;
    for (const auto &val : arr) {
        const QJsonObject o = val.toObject();
        v.append({static_cast<Palo>(o["palo"].toInt()), o["valor"].toInt(), o["boca"].toBool()});
    }
    return v;
}

inline QJsonArray montonesToJson(const QVector<Monton> &montones)
{
    QJsonArray arr;
    for (const auto &m : montones)
        arr.append(cartasToJson(m.cartas));
    return arr;
}

inline QVector<Monton> montonesFromJson(const QJsonArray &arr)
{
    QVector<Monton> v;
    for (const auto &val : arr) {
        Monton m;
        m.cartas = cartasFromJson(val.toArray());
        v.append(m);
    }
    return v;
}

inline QJsonObject estadoToJson(const EstadoMesa &e)
{
    QJsonObject o;
    o["stock"] = cartasToJson(e.stock.cartas);
    o["waste"] = cartasToJson(e.waste.cartas);
    o["columnas"] = montonesToJson(e.columnas);
    o["foundations"] = montonesToJson(e.foundations);
    o["celdas"] = cartasToJson(e.celdas);
    return o;
}

inline EstadoMesa estadoFromJson(const QJsonObject &o)
{
    EstadoMesa e;
    e.stock.cartas = cartasFromJson(o["stock"].toArray());
    e.waste.cartas = cartasFromJson(o["waste"].toArray());
    e.columnas = montonesFromJson(o["columnas"].toArray());
    e.foundations = montonesFromJson(o["foundations"].toArray());
    e.celdas = cartasFromJson(o["celdas"].toArray());
    return e;
}

} // namespace SerialMesa

class Historial {
public:
    static constexpr int LIMITE = 500;

    void limpiar()
    {
        m_deshacer.clear();
        m_rehacer.clear();
    }

    // Guarda la foto previa a una accion; una accion nueva invalida "rehacer"
    void registrar(const EstadoMesa &antes)
    {
        m_deshacer.append(antes);
        if (m_deshacer.size() > LIMITE)
            m_deshacer.removeFirst();
        m_rehacer.clear();
    }

    bool hayDeshacer() const { return !m_deshacer.isEmpty(); }
    bool hayRehacer() const { return !m_rehacer.isEmpty(); }

    // Devuelve el estado a restaurar; 'actual' pasa a la otra pila
    bool deshacer(const EstadoMesa &actual, EstadoMesa &restaurar)
    {
        if (m_deshacer.isEmpty()) return false;
        m_rehacer.append(actual);
        restaurar = m_deshacer.takeLast();
        return true;
    }

    bool rehacer(const EstadoMesa &actual, EstadoMesa &restaurar)
    {
        if (m_rehacer.isEmpty()) return false;
        m_deshacer.append(actual);
        restaurar = m_rehacer.takeLast();
        return true;
    }

    QJsonObject toJson() const
    {
        QJsonArray d, r;
        for (const auto &e : m_deshacer) d.append(SerialMesa::estadoToJson(e));
        for (const auto &e : m_rehacer) r.append(SerialMesa::estadoToJson(e));
        QJsonObject o;
        o["deshacer"] = d;
        o["rehacer"] = r;
        return o;
    }

    void fromJson(const QJsonObject &o)
    {
        limpiar();
        for (const auto &v : o["deshacer"].toArray()) m_deshacer.append(SerialMesa::estadoFromJson(v.toObject()));
        for (const auto &v : o["rehacer"].toArray()) m_rehacer.append(SerialMesa::estadoFromJson(v.toObject()));
    }

private:
    QVector<EstadoMesa> m_deshacer;
    QVector<EstadoMesa> m_rehacer;
};

// Regla de "movimiento seguro" a la fundacion: un As o un 2 siempre; otra
// carta solo si las dos fundaciones del color contrario ya llegan a su valor-1,
// porque entonces ninguna carta del tablero la necesitara como apoyo.
inline bool EsSeguraParaFundacion(const Carta &c, const QVector<Monton> &foundations)
{
    if (c.valor <= 2) return true;
    int tope[4] = {0, 0, 0, 0};
    for (const auto &f : foundations)
        if (!f.Vacia())
            tope[static_cast<int>(f.Top().palo)] = qMax(tope[static_cast<int>(f.Top().palo)], f.Top().valor);
    const bool roja = c.EsRojo();
    for (int p = 0; p < 4; ++p) {
        const Carta muestra{static_cast<Palo>(p), 1, true};
        if (muestra.EsRojo() != roja && tope[p] < c.valor - 1)
            return false;
    }
    return true;
}

// Indice de la fundacion donde puede ir la carta, o -1
inline int FundacionPara(const Carta &c, const QVector<Monton> &foundations)
{
    for (int i = 0; i < foundations.size(); ++i) {
        const auto &f = foundations[i];
        if (!f.Vacia() && f.Top().palo == c.palo && f.Top().valor == c.valor - 1)
            return i;
    }
    if (c.valor == 1)
        for (int i = 0; i < foundations.size(); ++i)
            if (foundations[i].Vacia())
                return i;
    return -1;
}

#endif // HISTORIAL_H
