#include "./src/utils/Semantic/SemanticAnalyzer.h"
#include <QDebug>

SemanticAnalyzer::SemanticAnalyzer() : currentPosition(0) {
    symbolTable = new SymbolTable();
}

SemanticAnalyzer::~SemanticAnalyzer() {
    delete symbolTable;
}

void SemanticAnalyzer::setTokens(const QVector<Token>& toks) {
    tokens = toks;
    currentPosition = 0;
}

void SemanticAnalyzer::reset() {
    symbolTable->clear();
    errors.clear();
    warnings.clear();
    currentPosition = 0;
    discoveredSymbols.clear();
}

bool SemanticAnalyzer::analyzeProgram() {
    reset();

    while (!isAtEnd()) {
        analyzeStatement();
    }

    // Check for unused variables
    QVector<Symbol> allSymbols = symbolTable->getDiscoveredSymbols();
    for (const auto& symbol : allSymbols) {
        if (!symbol.isInitialized) {
            addWarning(QString("Variable '%1' declared but never initialized")
                           .arg(symbol.name), symbol.line);
        }
    }

    return !hasErrors();
}

void SemanticAnalyzer::analyzeStatement() {
    Token tok = peek();

    if (tok.getType() == TokenType::KEYWORD) {
        QString keyword = tok.getLexeme().toLower();
        if (isTypeKeyword(keyword)) {
            if (isFunctionDeclaration()) {
                analyzeFunctionDeclaration();
            } else {
                analyzeDeclaration();
            }
        } else if (keyword == "if") {
            analyzeIfStatement();
        } else {
            advance(); 
        }
    }
    else if (tok.getType() == TokenType::IDENTIFIER) {
        if (peekNext().getType() == TokenType::ASSIGN) {
            analyzeAssignment();
        } else {
            if (!symbolTable->exists(tok.getLexeme())) {
                addError(QString("Undeclared identifier '%1'").arg(tok.getLexeme()), tok.getLine());
            }
            advance();
        }
    }
    else if (tok.getType() == TokenType::LBRACE) { 
        symbolTable->enterScope();
        advance();
        while(!isAtEnd() && !check(TokenType::RBRACE)) {
            analyzeStatement();
        }
        expect(TokenType::RBRACE, "Expected '}' to close block.");
        symbolTable->exitScope();
    }
    else if (tok.getType() == TokenType::SEMICOLON) {
        advance();
    }
    else {
        advance(); 
    }
}

void SemanticAnalyzer::analyzeFunctionDeclaration() {
    Token returnType = advance(); 
    Token funcName = advance();   

    expect(TokenType::LPAREN, "Expected '(' after function name.");

    symbolTable->enterScope();

    while (!isAtEnd() && !check(TokenType::RPAREN)) {
        if (isTypeKeyword(peek().getLexeme())) {
            Token paramType = advance();
            if (check(TokenType::IDENTIFIER)) {
                Token paramName = advance();
                Symbol paramSymbol(paramName.getLexeme(), SymbolTable::stringToType(paramType.getLexeme()), symbolTable->getCurrentScope(), paramName.getLine());
                if(symbolTable->addSymbol(paramSymbol)) {
                    discoveredSymbols.append(paramSymbol);
                }
            } else {
                addError("Expected parameter name after type", paramType.getLine());
            }
        } else {
            addError("Expected parameter type", peek().getLine());
            break; 
        }

        if (!check(TokenType::RPAREN)) {
            if (!match(TokenType::COMMA)) {
                addError("Expected ',' or ')' in parameter list", peek().getLine());
                break;
            }
        }
    }

    expect(TokenType::RPAREN, "Expected ')' to close parameter list.");

    if (check(TokenType::LBRACE)) {
        analyzeStatement(); 
    } else {
        addError("Expected '{' after function signature", peek().getLine());
    }

    symbolTable->exitScope(); 
}


void SemanticAnalyzer::analyzeDeclaration() {
    // Pattern: type identifier = value ;
    Token typeTok = advance();
    SymbolType type = SymbolTable::stringToType(typeTok.getLexeme());

    if (!check(TokenType::IDENTIFIER)) {
        addError("Expected identifier after type declaration", getCurrentLine());
        return;
    }

    Token idTok = advance();
    QString varName = idTok.getLexeme();
    int line = idTok.getLine();

    // Check if already declared in current scope
    if (symbolTable->existsInCurrentScope(varName)) {
        addError(QString("Variable '%1' already declared in this scope").arg(varName), line);
        return;
    }

    Symbol symbol(varName, type, symbolTable->getCurrentScope(), line);

    // Check for initialization
    if (check(TokenType::ASSIGN)) {
        advance(); // consume =

        if (!isAtEnd()) {
            Token valueTok = peek();
            SymbolType valueType = inferType(valueTok);

            if (valueType == SymbolType::UNKNOWN && valueTok.getType() == TokenType::IDENTIFIER) {
                const Symbol* refSym = symbolTable->lookup(valueTok.getLexeme());
                if (refSym) {
                    valueType = refSym->type;
                }
            }

            if (!isTypeCompatible(type, valueType)) {
                addError(QString("Type mismatch: cannot assign %1 to %2")
                             .arg(SymbolTable::typeToString(valueType))
                             .arg(SymbolTable::typeToString(type)), line);
            } else {
                QString valueStr;
                while (!isAtEnd() && !check(TokenType::SEMICOLON)) {
                    Token t = advance();
                    valueStr += t.getLexeme() + " ";
                }
                symbol.value = valueStr.trimmed();
                symbol.isInitialized = true;
            }
        }
    }

    if(symbolTable->addSymbol(symbol)) {
        discoveredSymbols.append(symbol);
    }

    if (check(TokenType::SEMICOLON)) {
        advance();
    }
}

void SemanticAnalyzer::analyzeAssignment() {
    Token idTok = advance();
    QString varName = idTok.getLexeme();
    int line = idTok.getLine();

    if (!symbolTable->exists(varName)) {
        addError(QString("Undeclared variable '%1'").arg(varName), line);
        advance(); 
        if (!isAtEnd()) advance(); 
        return;
    }

    Symbol* symbol = symbolTable->lookup(varName);

    if (!match(TokenType::ASSIGN)) {
        addError("Expected '=' in assignment", line);
        return;
    }

    if (isAtEnd()) {
        addError("Expected value after '='", line);
        return;
    }

    Token valueTok = advance();
    SymbolType valueType = inferType(valueTok);

    if (!isTypeCompatible(symbol->type, valueType)) {
        addError(QString("Type mismatch: cannot assign %1 to %2 variable '%3'")
                     .arg(SymbolTable::typeToString(valueType))
                     .arg(SymbolTable::typeToString(symbol->type))
                     .arg(varName), line);
    } else {
        symbolTable->updateSymbol(varName, valueTok.getLexeme());
        // Update discovered symbol as well
        for(auto& sym : discoveredSymbols) {
            if(sym.name == varName && sym.scope == symbol->scope) {
                sym.isInitialized = true;
                sym.value = valueTok.getLexeme();
                break;
            }
        }
    }

    if (check(TokenType::SEMICOLON)) {
        advance();
    }
}

void SemanticAnalyzer::analyzeExpression() {
    // Basic expression analysis
    while (!isAtEnd() && !check(TokenType::SEMICOLON) && !check(TokenType::RPAREN)) {
        Token tok = advance();

        if (tok.getType() == TokenType::IDENTIFIER) {
            if (!symbolTable->exists(tok.getLexeme())) {
                addError(QString("Undeclared identifier '%1'").arg(tok.getLexeme()),
                         tok.getLine());
            }
        }
    }
}

SymbolType SemanticAnalyzer::inferType(const Token& token) {
    switch (token.getType()) {
    case TokenType::INTEGER_LITERAL:
        return SymbolType::INTEGER;

    case TokenType::FLOAT_LITERAL:
        return SymbolType::FLOAT;

    case TokenType::STRING_LITERAL:
        return SymbolType::STRING;

    case TokenType::CHAR_LITERAL:
        return SymbolType::CHAR;

    case TokenType::LOGICAL_NOT:
        return SymbolType::BOOLEAN;

    case TokenType::KEYWORD: {
        QString keyword = token.getLexeme().toLower();
        if (keyword == "true" || keyword == "false") {
            return SymbolType::BOOLEAN;
        }
        return SymbolType::UNKNOWN;
    }

    case TokenType::IDENTIFIER: {
        const Symbol* sym = symbolTable->lookup(token.getLexeme());
        return sym ? sym->type : SymbolType::UNKNOWN;
    }

    default:
        return SymbolType::UNKNOWN;
    }
}

bool SemanticAnalyzer::isTypeCompatible(SymbolType expected, SymbolType actual) {
    if (expected == actual) return true;

    // Allow implicit conversions
    if (expected == SymbolType::FLOAT && actual == SymbolType::INTEGER) return true;
    if (expected == SymbolType::DOUBLE &&
        (actual == SymbolType::INTEGER || actual == SymbolType::FLOAT)) return true;
    if (expected == SymbolType::STRING && actual == SymbolType::CHAR) return true;

    return false;
}

// Token navigation methods
Token SemanticAnalyzer::peek() const {
    if (currentPosition >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentPosition];
}

Token SemanticAnalyzer::peekNext() const {
    if (currentPosition + 1 >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentPosition + 1];
}

Token SemanticAnalyzer::advance() {
    if (currentPosition >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentPosition++];
}

bool SemanticAnalyzer::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool SemanticAnalyzer::expect(TokenType type, const QString& message) {
    if (check(type)) {
        advance();
        return true;
    }
    addError(message, getCurrentLine());
    return false;
}

bool SemanticAnalyzer::check(TokenType type) const {
    if (currentPosition >= tokens.size()) return false;
    return tokens[currentPosition].getType() == type;
}

bool SemanticAnalyzer::isAtEnd() const {
    return currentPosition >= tokens.size();
}

void SemanticAnalyzer::addError(const QString& message, int line) {
    if (line < 0) line = getCurrentLine();
    errors.append(SemanticError(message, line, "Error"));
}

void SemanticAnalyzer::addWarning(const QString& message, int line) {
    if (line < 0) line = getCurrentLine();
    warnings.append(SemanticError(message, line, "Warning"));
}

int SemanticAnalyzer::getCurrentLine() const {
    if (currentPosition >= tokens.size()) return 0;
    return tokens[currentPosition].getLine();
}

void SemanticAnalyzer::analyzeIfStatement() {
    advance(); // consume 'if'

    expect(TokenType::LPAREN, "Expected '(' after 'if'.");

    analyzeExpression();

    expect(TokenType::RPAREN, "Expected ')' after if condition.");

    if (check(TokenType::LBRACE)) {
        symbolTable->enterScope();
        advance(); // consume '{'
        while (!isAtEnd() && !check(TokenType::RBRACE)) {
            Token tok = peek();
            if (tok.getType() == TokenType::KEYWORD && (tok.getLexeme() == "int" || tok.getLexeme() == "bool")) {
                analyzeDeclaration();
            } else {
                advance();
            }
        }
        match(TokenType::RBRACE);
        symbolTable->exitScope();
    }
}

bool SemanticAnalyzer::isTypeKeyword(const QString& keyword) const {
    QString lower = keyword.toLower();
    return lower == "int" || lower == "float" || lower == "string" ||
           lower == "bool" || lower == "char" || lower == "double" ||
           lower == "short" || lower == "long" || lower == "void";
}

bool SemanticAnalyzer::isFunctionDeclaration() const {
    if (peekNext().getType() == TokenType::IDENTIFIER &&
        peekAhead(2).getType() == TokenType::LPAREN) {
        return true;
    }
    return false;
}

Token SemanticAnalyzer::peekAhead(int offset) const {
    int pos = currentPosition + offset;
    if (pos >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[pos];
}

