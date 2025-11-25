#include "Parser.h"
#include <QDebug>

Parser::Parser() : grammar(nullptr), currentPosition(0) {}

Parser::Parser(Grammar* g) : grammar(g), currentPosition(0) {}

Parser::~Parser() {}

void Parser::setGrammar(Grammar* g) {
    grammar = g;
}

void Parser::setTokens(const QVector<Token>& toks) {
    tokens = toks;
    currentPosition = 0;
}

void Parser::reset() {
    currentPosition = 0;
    errors.clear();
}

ParseTree Parser::parse() {
    reset();

    if (!grammar) {
        errors.append(ParseError("No grammar set", 0, "", ""));
        return ParseTree();
    }

    return parseExpression();
}

ParseTree Parser::parseExpression() {
    ParseTree tree(grammar->getName());

    if (tokens.isEmpty()) {
        addError("Empty token stream", "tokens");
        return tree;
    }

    // Start parsing from the start symbol
    auto root = parseE();
    tree.setRoot(root);

    return tree;
}

std::shared_ptr<ParseTreeNode> Parser::parseE() {
    // E → T E'
    auto node = std::make_shared<ParseTreeNode>("E", false);

    auto tNode = parseT();
    if (tNode) {
        node->addChild(tNode);
    }

    auto ePrimeNode = parseEPrime();
    if (ePrimeNode) {
        node->addChild(ePrimeNode);
    }

    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseEPrime() {
    // E' → + T E' | ε
    auto node = std::make_shared<ParseTreeNode>("E'", false);

    if (check("+") || check("PLUS")) {
        advance();
        auto plusNode = std::make_shared<ParseTreeNode>("+", true);
        node->addChild(plusNode);

        auto tNode = parseT();
        if (tNode) {
            node->addChild(tNode);
        }

        auto ePrimeNode = parseEPrime();
        if (ePrimeNode) {
            node->addChild(ePrimeNode);
        }
    } else {
        // ε production
        auto epsilonNode = std::make_shared<ParseTreeNode>("ε", true);
        node->addChild(epsilonNode);
    }

    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseT() {
    // T → F T'
    auto node = std::make_shared<ParseTreeNode>("T", false);

    auto fNode = parseF();
    if (fNode) {
        node->addChild(fNode);
    }

    auto tPrimeNode = parseTPrime();
    if (tPrimeNode) {
        node->addChild(tPrimeNode);
    }

    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseTPrime() {
    // T' → * F T' | ε
    auto node = std::make_shared<ParseTreeNode>("T'", false);

    if (check("*") || check("MULTIPLY")) {
        advance();
        auto mulNode = std::make_shared<ParseTreeNode>("*", true);
        node->addChild(mulNode);

        auto fNode = parseF();
        if (fNode) {
            node->addChild(fNode);
        }

        auto tPrimeNode = parseTPrime();
        if (tPrimeNode) {
            node->addChild(tPrimeNode);
        }
    } else {
        // ε production
        auto epsilonNode = std::make_shared<ParseTreeNode>("ε", true);
        node->addChild(epsilonNode);
    }

    return node;
}

std::shared_ptr<ParseTreeNode> Parser::parseF() {
    // F → ( E ) | id | num
    auto node = std::make_shared<ParseTreeNode>("F", false);

    if (check("(") || check("LPAREN")) {
        advance();
        auto lparenNode = std::make_shared<ParseTreeNode>("(", true);
        node->addChild(lparenNode);

        auto eNode = parseE();
        if (eNode) {
            node->addChild(eNode);
        }

        if (check(")") || check("RPAREN")) {
            advance();
            auto rparenNode = std::make_shared<ParseTreeNode>(")", true);
            node->addChild(rparenNode);
        } else {
            addError("Expected ')'", ")");
        }
    } else if (check("IDENTIFIER") || check("id")) {
        Token tok = advance();
        auto idNode = std::make_shared<ParseTreeNode>("id", tok.getLexeme(), true);
        node->addChild(idNode);
    } else if (check("INTEGER") || check("num") || check("INTEGER_LITERAL")) {
        Token tok = advance();
        auto numNode = std::make_shared<ParseTreeNode>("num", tok.getLexeme(), true);
        node->addChild(numNode);
    } else {
        addError("Expected '(', identifier, or number", "id | num | (");
    }

    return node;
}

Token Parser::peek() const {
    if (currentPosition >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentPosition];
}

Token Parser::advance() {
    if (currentPosition >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentPosition++];
}

bool Parser::match(const QString& expected) {
    if (check(expected)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(const QString& expected) const {
    if (currentPosition >= tokens.size()) return false;

    Token tok = peek();

    // Check by token type string
    if (tok.getTypeString() == expected) return true;

    // Check by lexeme
    if (tok.getLexeme() == expected) return true;

    return false;
}

bool Parser::isAtEnd() const {
    return currentPosition >= tokens.size();
}

void Parser::addError(const QString& message, const QString& expected) {
    errors.append(ParseError(message, currentPosition, expected, getCurrentTokenString()));
}

QString Parser::getCurrentTokenString() const {
    if (currentPosition >= tokens.size()) {
        return "EOF";
    }
    Token tok = tokens[currentPosition];
    return QString("%1 ('%2')").arg(tok.getTypeString()).arg(tok.getLexeme());
}
