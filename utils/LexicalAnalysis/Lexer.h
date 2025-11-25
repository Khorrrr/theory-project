#ifndef LEXER_H
#define LEXER_H

#include "./src/models/LexicalAnalysis/Token.h"
#include "./src/utils/LexicalAnalysis/AutomatonManager.h"
#include <QString>
#include <QVector>

struct LexerError {
    QString message;
    int line;
    int column;
    QString lexeme;

    LexerError(const QString& msg, int l, int c, const QString& lex)
        : message(msg), line(l), column(c), lexeme(lex) {}

    QString toString() const {
        return QString("Error at Line %1, Column %2: %3 (near '%4')")
        .arg(line).arg(column).arg(message).arg(lexeme);
    }
};

class Lexer {
private:
    QString input;
    int position;
    int line;
    int column;
    QVector<Token> tokens;
    QVector<LexerError> errors;
    AutomatonManager* automatonManager;
    bool skipWhitespace;
    bool skipComments;

public:
    Lexer();
    Lexer(AutomatonManager* manager);
    ~Lexer();

    void setAutomatonManager(AutomatonManager* manager);
    void setSkipWhitespace(bool skip) { skipWhitespace = skip; }
    void setSkipComments(bool skip) { skipComments = skip; }

    bool tokenize(const QString& sourceCode);
    QVector<Token> getTokens() const { return tokens; }
    QVector<LexerError> getErrors() const { return errors; }
    bool hasErrors() const { return !errors.isEmpty(); }

    QString getTokensString() const;
    QString getErrorsString() const;
    void reset();

private:
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;

    Token scanToken();
    Token recognizeWithAutomaton();
    Token recognizeNumber();
    Token recognizeString();
    Token recognizeChar();
    Token recognizeIdentifierOrKeyword();
    Token recognizeOperator();
    Token recognizeComment();

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isWhitespace(char c) const;

    void addError(const QString& message, const QString& lexeme = "");
    void skipWhitespaceChars();
};

#endif // LEXER_H
