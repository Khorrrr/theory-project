#ifndef PRODUCTION_H
#define PRODUCTION_H

#include <QString>
#include <QVector>

class Production {
private:
    QString nonTerminal;           // Left-hand side (e.g., "E")
    QVector<QString> symbols;      // Right-hand side (e.g., ["E", "+", "T"])

public:
    Production();
    Production(const QString& lhs, const QVector<QString>& rhs);

    QString getNonTerminal() const { return nonTerminal; }
    QVector<QString> getSymbols() const { return symbols; }

    void setNonTerminal(const QString& nt) { nonTerminal = nt; }
    void setSymbols(const QVector<QString>& syms) { symbols = syms; }

    QString toString() const;
    bool isEmpty() const;
    bool isEpsilon() const;

    static Production fromString(const QString& str);
};

#endif // PRODUCTION_H
