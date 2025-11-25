#include "Token.h"
#include <QMap>
#include <QSet>

Token::Token()
    : type(TokenType::UNKNOWN), lexeme(""), automatonId(""), line(0), column(0) {}

Token::Token(TokenType type, const QString& lexeme, int line, int column, const QString& automatonId)
    : type(type), lexeme(lexeme), automatonId(automatonId), line(line), column(column) {}

QString Token::getTypeString() const {
    return tokenTypeToString(type);
}

QString Token::toString() const {
    return QString("Token(%1, \"%2\", Line: %3, Col: %4)")
    .arg(getTypeString()).arg(lexeme).arg(line).arg(column);
}

bool Token::isValid() const {
    return type != TokenType::UNKNOWN && type != TokenType::END_OF_FILE;
}

QString Token::tokenTypeToString(TokenType type) {
    static QMap<TokenType, QString> typeNames = {
        {TokenType::KEYWORD, "KEYWORD"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::INTEGER_LITERAL, "INTEGER"},
        {TokenType::FLOAT_LITERAL, "FLOAT"},
        {TokenType::STRING_LITERAL, "STRING"},
        {TokenType::CHAR_LITERAL, "CHAR"},
        {TokenType::PLUS, "PLUS"}, {TokenType::MINUS, "MINUS"},
        {TokenType::MULTIPLY, "MULTIPLY"}, {TokenType::DIVIDE, "DIVIDE"},
        {TokenType::MODULO, "MODULO"}, {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::EQUAL, "EQUAL"}, {TokenType::NOT_EQUAL, "NOT_EQUAL"},
        {TokenType::LESS_THAN, "LESS_THAN"}, {TokenType::GREATER_THAN, "GREATER_THAN"},
        {TokenType::LESS_EQUAL, "LESS_EQUAL"}, {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
        {TokenType::LOGICAL_AND, "AND"}, {TokenType::LOGICAL_OR, "OR"},
        {TokenType::LOGICAL_NOT, "NOT"}, {TokenType::BITWISE_AND, "BIT_AND"},
        {TokenType::BITWISE_OR, "BIT_OR"}, {TokenType::BITWISE_XOR, "BIT_XOR"},
        {TokenType::BITWISE_NOT, "BIT_NOT"}, {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::COMMA, "COMMA"}, {TokenType::DOT, "DOT"}, {TokenType::COLON, "COLON"},
        {TokenType::LPAREN, "LPAREN"}, {TokenType::RPAREN, "RPAREN"},
        {TokenType::LBRACE, "LBRACE"}, {TokenType::RBRACE, "RBRACE"},
        {TokenType::LBRACKET, "LBRACKET"}, {TokenType::RBRACKET, "RBRACKET"},
        {TokenType::WHITESPACE, "WHITESPACE"}, {TokenType::COMMENT, "COMMENT"},
        {TokenType::NEWLINE, "NEWLINE"}, {TokenType::UNKNOWN, "UNKNOWN"},
        {TokenType::END_OF_FILE, "EOF"}
    };
    return typeNames.value(type, "UNKNOWN");
}

bool Token::isKeyword(const QString& str) {
    static QSet<QString> keywords = {
        "if", "else", "while", "for", "do", "switch", "case", "default",
        "break", "continue", "return", "void", "int", "float", "double",
        "char", "bool", "true", "false", "const", "static", "class",
        "public", "private", "protected", "struct", "enum", "string"
    };
    return keywords.contains(str.toLower());
}

TokenType Token::getKeywordType(const QString& str) {
    return isKeyword(str) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
}

TokenType Token::getOperatorType(const QString& str) {
    static QMap<QString, TokenType> operators = {
        {"+", TokenType::PLUS}, {"-", TokenType::MINUS}, {"*", TokenType::MULTIPLY},
        {"/", TokenType::DIVIDE}, {"%", TokenType::MODULO}, {"=", TokenType::ASSIGN},
        {"==", TokenType::EQUAL}, {"!=", TokenType::NOT_EQUAL},
        {"<", TokenType::LESS_THAN}, {">", TokenType::GREATER_THAN},
        {"<=", TokenType::LESS_EQUAL}, {">=", TokenType::GREATER_EQUAL},
        {"&&", TokenType::LOGICAL_AND}, {"||", TokenType::LOGICAL_OR},
        {"!", TokenType::LOGICAL_NOT}, {"&", TokenType::BITWISE_AND},
        {"|", TokenType::BITWISE_OR}, {"^", TokenType::BITWISE_XOR},
        {"~", TokenType::BITWISE_NOT}, {";", TokenType::SEMICOLON},
        {",", TokenType::COMMA}, {".", TokenType::DOT}, {":", TokenType::COLON},
        {"(", TokenType::LPAREN}, {")", TokenType::RPAREN},
        {"{", TokenType::LBRACE}, {"}", TokenType::RBRACE},
        {"[", TokenType::LBRACKET}, {"]", TokenType::RBRACKET}
    };
    return operators.value(str, TokenType::UNKNOWN);
}
