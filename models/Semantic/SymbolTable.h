#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <QString>
#include <QMap>
#include <QVector>

enum class SymbolType {
    INTEGER,
    FLOAT,
    DOUBLE,
    CHAR,
    STRING,
    BOOLEAN,
    VOID,
    FUNCTION,
    UNKNOWN
};

struct Symbol {
    QString name;
    SymbolType type;
    QString value;
    int scope;
    int line;
    bool isInitialized;
    bool isConstant;

    Symbol() : type(SymbolType::UNKNOWN), scope(0), line(0),
        isInitialized(false), isConstant(false) {}

    Symbol(const QString& n, SymbolType t, int s = 0, int l = 0)
        : name(n), type(t), scope(s), line(l),
        isInitialized(false), isConstant(false) {}

    QString getTypeString() const;
    QString toString() const;
};

class SymbolTable {
private:
    QVector<QMap<QString, Symbol>> scopes;
    int currentScope;
    QVector<Symbol> allDiscoveredSymbols;

public:
    SymbolTable();

    void enterScope();
    void exitScope();
    int getCurrentScope() const { return currentScope; }

    bool addSymbol(const Symbol& symbol);
    bool updateSymbol(const QString& name, const QString& value);
    Symbol* lookup(const QString& name);
    const Symbol* lookup(const QString& name) const;

    bool exists(const QString& name) const;
    bool existsInCurrentScope(const QString& name) const;

    QVector<Symbol> getDiscoveredSymbols() const { return allDiscoveredSymbols; }
    QVector<Symbol> getSymbolsInScope(int scope) const;

    void clear();
    QString toString() const;

    static SymbolType stringToType(const QString& typeStr);
    static QString typeToString(SymbolType type);
};

#endif // SYMBOLTABLE_H
