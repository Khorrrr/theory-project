#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include <QVector>
#include <QString>
#include "../models/LexicalAnalysis/Token.h"
#include "../models/Semantic/SymbolTable.h"

// SemanticError struct defined directly in the header
struct SemanticError {
    QString message;
    int line;
    QString type; // "Error" or "Warning"

    SemanticError(const QString& msg = "", int ln = 0, const QString& t = "Error")
        : message(msg), line(ln), type(t) {}

    // ADD THIS METHOD
    QString toString() const {
        return QString("Line %1: %2").arg(line).arg(message);
    }
};

class SemanticAnalyzer {
private:
    QVector<Token> tokens;
    SymbolTable* symbolTable;
    int currentPosition;
    QVector<SemanticError> errors;
    QVector<SemanticError> warnings;
    QVector<Symbol> discoveredSymbols;

    // Private methods
    void analyzeStatement();
    void analyzeDeclaration();
    void analyzeAssignment();
    void analyzeExpression();
    void analyzeIfStatement();
    void analyzeFunctionDeclaration();
    SymbolType inferType(const Token& token);
    bool isTypeCompatible(SymbolType expected, SymbolType actual);
    bool isFunctionDeclaration() const;
    bool isTypeKeyword(const QString& keyword) const;

    // Token navigation methods
    Token peek() const;
    Token peekNext() const;
    Token peekAhead(int offset) const;
    Token advance();
    bool match(TokenType type);
    bool expect(TokenType type, const QString& message);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    void addError(const QString& message, int line = -1);
    void addWarning(const QString& message, int line = -1);
    int getCurrentLine() const;

public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    void setTokens(const QVector<Token>& toks);
    void reset();
    bool analyzeProgram();

    // ADD THESE PUBLIC METHODS
    SymbolTable* getSymbolTable() const { return symbolTable; }
    QVector<Symbol> getDiscoveredSymbols() const { return discoveredSymbols; }
    bool hasErrors() const { return !errors.isEmpty(); }
    QVector<SemanticError> getErrors() const { return errors; }
    QVector<SemanticError> getWarnings() const { return warnings; }
};

#endif // SEMANTICANALYZER_H
