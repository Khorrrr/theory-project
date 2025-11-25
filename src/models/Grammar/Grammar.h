#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "Production.h"
#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>

class Grammar {
private:
    QString name;
    QString startSymbol;
    QVector<Production> productions;
    QSet<QString> terminals;
    QSet<QString> nonTerminals;

public:
    Grammar();
    Grammar(const QString& name, const QString& start);

    QString getName() const { return name; }
    QString getStartSymbol() const { return startSymbol; }
    QVector<Production> getProductions() const { return productions; }
    QSet<QString> getTerminals() const { return terminals; }
    QSet<QString> getNonTerminals() const { return nonTerminals; }

    void setName(const QString& n) { name = n; }
    void setStartSymbol(const QString& s) { startSymbol = s; }

    bool addProduction(const Production& prod);
    bool removeProduction(int index);
    void clear();

    void addTerminal(const QString& terminal);
    void addNonTerminal(const QString& nonTerminal);

    QVector<Production> getProductionsFor(const QString& nonTerminal) const;

    bool isTerminal(const QString& symbol) const;
    bool isNonTerminal(const QString& symbol) const;

    QString toString() const;

    // Predefined grammars
    static Grammar createArithmeticGrammar();
    static Grammar createSimpleStatementGrammar();
    static Grammar createExpressionGrammar();
};

#endif // GRAMMAR_H
