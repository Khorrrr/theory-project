#include "Lexer.h"
#include <QDebug>

Lexer::Lexer()
    : position(0), line(1), column(1), automatonManager(nullptr),
    skipWhitespace(true), skipComments(true) {}

Lexer::Lexer(AutomatonManager* manager)
    : position(0), line(1), column(1), automatonManager(manager),
    skipWhitespace(true), skipComments(true) {}

Lexer::~Lexer() {}

void Lexer::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
}

bool Lexer::tokenize(const QString& sourceCode) {
    reset();
    input = sourceCode;

    while (!isAtEnd()) {
        Token token = scanToken();

        if (token.getType() == TokenType::WHITESPACE && skipWhitespace) continue;
        if (token.getType() == TokenType::COMMENT && skipComments) continue;
        if (token.getType() != TokenType::UNKNOWN) {
            tokens.push_back(token);
        }
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
    return !hasErrors();
}

void Lexer::reset() {
    input.clear();
    position = 0;
    line = 1;
    column = 1;
    tokens.clear();
    errors.clear();
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : input[position].toLatin1();
}

char Lexer::peekNext() const {
    return (position + 1 >= input.length()) ? '\0' : input[position + 1].toLatin1();
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char c = input[position].toLatin1();
    position++;
    column++;
    if (c == '\n') {
        line++;
        column = 1;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return position >= input.length();
}

Token Lexer::scanToken() {
    skipWhitespaceChars();
    if (isAtEnd()) return Token(TokenType::END_OF_FILE, "", line, column);

    int startLine = line;
    int startColumn = column;
    char c = peek();

    if (c == '/' && peekNext() == '/') return recognizeComment();
    if (c == '"') return recognizeString();
    if (c == '\'') return recognizeChar();
    if (isDigit(c)) return recognizeNumber();
    if (isAlpha(c) || c == '_') return recognizeIdentifierOrKeyword();

    Token opToken = recognizeOperator();
    if (opToken.getType() != TokenType::UNKNOWN) return opToken;

    if (automatonManager) {
        Token autoToken = recognizeWithAutomaton();
        if (autoToken.getType() != TokenType::UNKNOWN) return autoToken;
    }

    QString lexeme = QString(c);
    advance();
    addError("Unexpected character", lexeme);
    return Token(TokenType::UNKNOWN, lexeme, startLine, startColumn);
}

Token Lexer::recognizeWithAutomaton() {
    int startPos = position;
    int startLine = line;
    int startColumn = column;

    QString longestMatch;
    QString matchingAutomatonId;
    int longestMatchEnd = position;

    for (int end = position + 1; end <= input.length(); ++end) {
        QString substring = input.mid(position, end - position);
        if (automatonManager) {
            QString automatonId = automatonManager->findMatchingAutomaton(substring);
            if (!automatonId.isEmpty()) {
                longestMatch = substring;
                matchingAutomatonId = automatonId;
                longestMatchEnd = end;
            }
        }
    }

    if (!longestMatch.isEmpty()) {
        while (position < longestMatchEnd) advance();

        TokenType type = TokenType::UNKNOWN;
        if (matchingAutomatonId == "IDENTIFIER") {
            type = Token::getKeywordType(longestMatch);
        } else if (matchingAutomatonId == "INTEGER") {
            type = TokenType::INTEGER_LITERAL;
        } else if (matchingAutomatonId == "FLOAT") {
            type = TokenType::FLOAT_LITERAL;
        }

        return Token(type, longestMatch, startLine, startColumn, matchingAutomatonId);
    }

    return Token(TokenType::UNKNOWN, "", startLine, startColumn);
}

Token Lexer::recognizeNumber() {
    int startLine = line;
    int startColumn = column;
    QString number;
    bool isFloat = false;

    while (isDigit(peek())) number += advance();

    if (peek() == '.' && isDigit(peekNext())) {
        isFloat = true;
        number += advance();
        while (isDigit(peek())) number += advance();
    }

    TokenType type = isFloat ? TokenType::FLOAT_LITERAL : TokenType::INTEGER_LITERAL;
    QString automatonId = isFloat ? "FLOAT" : "INTEGER";
    return Token(type, number, startLine, startColumn, automatonId);
}

Token Lexer::recognizeString() {
    int startLine = line;
    int startColumn = column;
    QString str;
    str += advance();

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            addError("Unterminated string literal", str);
            return Token(TokenType::UNKNOWN, str, startLine, startColumn);
        }
        if (peek() == '\\') {
            str += advance();
            if (!isAtEnd()) str += advance();
        } else {
            str += advance();
        }
    }

    if (isAtEnd()) {
        addError("Unterminated string literal", str);
        return Token(TokenType::UNKNOWN, str, startLine, startColumn);
    }

    str += advance();
    return Token(TokenType::STRING_LITERAL, str, startLine, startColumn);
}

Token Lexer::recognizeChar() {
    int startLine = line;
    int startColumn = column;
    QString ch;
    ch += advance();

    if (isAtEnd() || peek() == '\'') {
        addError("Empty character literal", ch);
        return Token(TokenType::UNKNOWN, ch, startLine, startColumn);
    }

    if (peek() == '\\') {
        ch += advance();
        if (!isAtEnd()) ch += advance();
    } else {
        ch += advance();
    }

    if (peek() != '\'') {
        addError("Unterminated character literal", ch);
        return Token(TokenType::UNKNOWN, ch, startLine, startColumn);
    }

    ch += advance();
    return Token(TokenType::CHAR_LITERAL, ch, startLine, startColumn);
}

Token Lexer::recognizeIdentifierOrKeyword() {
    int startLine = line;
    int startColumn = column;
    QString identifier;

    while (isAlphaNumeric(peek()) || peek() == '_') {
        identifier += advance();
    }

    TokenType type = Token::getKeywordType(identifier);
    return Token(type, identifier, startLine, startColumn, "IDENTIFIER");
}

Token Lexer::recognizeOperator() {
    int startLine = line;
    int startColumn = column;
    char c = peek();
    char next = peekNext();

    QString twoChar = QString(c) + QString(next);
    TokenType twoCharType = Token::getOperatorType(twoChar);

    if (twoCharType != TokenType::UNKNOWN) {
        advance();
        advance();
        return Token(twoCharType, twoChar, startLine, startColumn);
    }

    QString oneChar = QString(c);
    TokenType oneCharType = Token::getOperatorType(oneChar);

    if (oneCharType != TokenType::UNKNOWN) {
        advance();
        return Token(oneCharType, oneChar, startLine, startColumn);
    }

    return Token(TokenType::UNKNOWN, "", startLine, startColumn);
}

Token Lexer::recognizeComment() {
    int startLine = line;
    int startColumn = column;
    QString comment;

    comment += advance();
    comment += advance();

    while (!isAtEnd() && peek() != '\n') {
        comment += advance();
    }

    return Token(TokenType::COMMENT, comment, startLine, startColumn);
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void Lexer::skipWhitespaceChars() {
    while (!isAtEnd() && isWhitespace(peek())) {
        advance();
    }
}

void Lexer::addError(const QString& message, const QString& lexeme) {
    errors.push_back(LexerError(message, line, column, lexeme));
}

QString Lexer::getTokensString() const {
    QString result;
    for (const auto& token : tokens) {
        result += token.toString() + "\n";
    }
    return result;
}

QString Lexer::getErrorsString() const {
    QString result;
    for (const auto& error : errors) {
        result += error.toString() + "\n";
    }
    return result;
}
