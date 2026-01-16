#ifndef CARTA_H
#define CARTA_H

#include <QString>

enum class Palo { Trebol, Diamante, Corazon, Espada };

struct Carta {
    Palo palo;
    int valor; // 1-13
    bool bocaArriba = false;

    bool EsVacia() const { return valor == 0; }

    QString PaloStr() const {
        switch (palo) {
        case Palo::Trebol:   return QString::fromUtf8("\u2663");
        case Palo::Diamante: return QString::fromUtf8("\u2666");
        case Palo::Corazon:  return QString::fromUtf8("\u2665");
        case Palo::Espada:   return QString::fromUtf8("\u2660");
        default:             return "";
        }
    }

    bool EsRojo() const { return palo == Palo::Corazon || palo == Palo::Diamante; }

    QString ValorStr() const {
        switch (valor) {
        case 1:  return "A";
        case 11: return "J";
        case 12: return "Q";
        case 13: return "K";
        default: return QString::number(valor);
        }
    }
};

#endif