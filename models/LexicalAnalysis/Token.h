#ifndef TOKEN_H
#define TOKEN_H

#include <QString>
#include <QMetaType>

enum class TokenType {
    // Keywords
    KEYWORD,

    // Identifiers and Literals
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,

    // Operators
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    ASSIGN, EQUAL, NOT_EQUAL,
    LESS_THAN, GREATER_THAN, LESS_EQUAL, GREATER_EQUAL,
    LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT,
    BITWISE_AND, BITWISE_OR, BITWISE_XOR, BITWISE_NOT,

    // Delimiters
    SEMICOLON, COMMA, DOT, COLON,

    // Brackets
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,

    // Special
    WHITESPACE, COMMENT, NEWLINE,

    // Error and End
    UNKNOWN, END_OF_FILE
};

class Token {
private:
    TokenType type;
    QString lexeme;
    QString automatonId;
    int line;
    int column;

public:
    Token();
    Token(TokenType type, const QString& lexeme, int line = 0, int column = 0, const QString& automatonId = "");

    TokenType getType() const { return type; }
    QString getLexeme() const { return lexeme; }
    QString getAutomatonId() const { return automatonId; }
    int getLine() const { return line; }
    int getColumn() const { return column; }

    void setType(TokenType t) { type = t; }
    void setLexeme(const QString& lex) { lexeme = lex; }
    void setAutomatonId(const QString& id) { automatonId = id; }
    void setLine(int l) { line = l; }
    void setColumn(int c) { column = c; }

    QString getTypeString() const;
    QString toString() const;
    bool isValid() const;

    static QString tokenTypeToString(TokenType type);
    static bool isKeyword(const QString& str);
    static TokenType getKeywordType(const QString& str);
    static TokenType getOperatorType(const QString& str);
};

Q_DECLARE_METATYPE(Token)

#endif // TOKEN_H
