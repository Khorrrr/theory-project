#ifndef PARSER_H
#define PARSER_H

#include "./src/models/Grammar/Grammar.h"
#include "./src/models/Grammar/ParseTree.h"
#include "./src/models/LexicalAnalysis/Token.h"
#include <QVector>
#include <QString>

struct ParseError {
    QString message;
    int position;
    QString expected;
    QString found;

    ParseError(const QString& msg, int pos, const QString& exp, const QString& fnd)
        : message(msg), position(pos), expected(exp), found(fnd) {}

    QString toString() const {
        return QString("Parse Error at position %1: %2\nExpected: %3\nFound: %4")
        .arg(position).arg(message).arg(expected).arg(found);
    }
};

class Parser {
private:
    Grammar* grammar;
    QVector<Token> tokens;
    int currentPosition;
    QVector<ParseError> errors;
    std::shared_ptr<ParseTreeNode> currentNode;

public:
    Parser();
    Parser(Grammar* g);
    ~Parser();

    void setGrammar(Grammar* g);
    void setTokens(const QVector<Token>& toks);

    ParseTree parse();
    ParseTree parseExpression();  // Specific for expression grammar

    QVector<ParseError> getErrors() const { return errors; }
    bool hasErrors() const { return !errors.isEmpty(); }

    void reset();

private:
    // Recursive descent parsing functions
    std::shared_ptr<ParseTreeNode> parseE();      // Expression
    std::shared_ptr<ParseTreeNode> parseEPrime(); // E'
    std::shared_ptr<ParseTreeNode> parseT();      // Term
    std::shared_ptr<ParseTreeNode> parseTPrime(); // T'
    std::shared_ptr<ParseTreeNode> parseF();      // Factor

    // Helper functions
    Token peek() const;
    Token advance();
    bool match(const QString& expected);
    bool check(const QString& expected) const;
    bool isAtEnd() const;

    void addError(const QString& message, const QString& expected);
    QString getCurrentTokenString() const;
};

#endif // PARSER_H
