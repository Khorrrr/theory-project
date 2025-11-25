#include "CodeGenerator.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QCoreApplication>

CodeGenerator::CodeGenerator()
    : symbolTable(nullptr), targetLanguage(TargetLanguage::PYTHON),
    indentLevel(0), currentPosition(0), labelCounter(0), inGlobalScope(true) {}

CodeGenerator::~CodeGenerator() {}

void CodeGenerator::setTokens(const QVector<Token>& toks) {
    tokens = toks;
    currentPosition = 0;
    optimizeTokens();
}

void CodeGenerator::setSymbolTable(SymbolTable* table) {
    symbolTable = table;
}

void CodeGenerator::setTargetLanguage(TargetLanguage lang) {
    targetLanguage = lang;
}




void CodeGenerator::setSourceCode(const QString& source) {
    m_sourceCode = source;
}

void CodeGenerator::reset() {
    generatedCode.clear();
    indentLevel = 0;
    currentPosition = 0;
    labelCounter = 0;
    inGlobalScope = true;
}

QString CodeGenerator::generate() {
    reset();



    switch (targetLanguage) {
    case TargetLanguage::PYTHON:
        return translateToPython();
    case TargetLanguage::JAVA:
        return translateToJava();
    case TargetLanguage::JAVASCRIPT:
        return translateToJavaScript();
    case TargetLanguage::ASSEMBLY:
        return translateToAssembly();
    default:
        return "// Error: Unsupported Language";
    }
}




void CodeGenerator::optimizeTokens() {
    if (tokens.isEmpty()) return;

    QVector<Token> optimized;
    bool changesMade = false;

    for (int i = 0; i < tokens.size(); ++i) {
        if (i + 2 < tokens.size() &&
            tokens[i].getType() == TokenType::INTEGER_LITERAL &&
            tokens[i+1].getType() == TokenType::PLUS &&
            tokens[i+2].getType() == TokenType::INTEGER_LITERAL) {

            int v1 = tokens[i].getLexeme().toInt();
            int v2 = tokens[i+2].getLexeme().toInt();
            int sum = v1 + v2;

            Token newToken(TokenType::INTEGER_LITERAL, QString::number(sum), 0, 0);
            optimized.append(newToken);

            i += 2;
            changesMade = true;
        }
        else {
            optimized.append(tokens[i]);
        }
    }

    tokens = optimized;

    if (changesMade) {
        optimizeTokens();
    }
}

// ============================================================
//  RULE-BASED TRANSLATORS
// ============================================================

QString CodeGenerator::translateToPython() {
    QString code = "# Generated Python Code\n\n";
    currentPosition = 0;

    while (!isAtEnd()) {
        try {
            processStatement(code);
        } catch (...) {
            synchronize();
        }
    }

    return code;
}

QString CodeGenerator::translateToJava() {
    QString code = "// Generated Java Code\n";
    code += "import java.util.*;\n\n";
    code += "public class Main {\n";
    indentLevel++;

    currentPosition = 0;
    bool hasMain = false;

    // First pass: look for main function
    while (!isAtEnd()) {
        if (isFunctionDeclaration()) {
            Token returnType = peek();
            advance(); // type
            Token funcName = peek();

            if (funcName.getLexeme() == "main") {
                hasMain = true;
                break;
            }
        }
        advance();
    }

    currentPosition = 0;

    // Generate functions
    while (!isAtEnd()) {
        try {
            processStatement(code);
        } catch (...) {
            synchronize();
        }
    }

    indentLevel--;
    code += "}\n";
    return code;
}

QString CodeGenerator::translateToJavaScript() {
    QString code = "// Generated JavaScript Code\n\n";
    currentPosition = 0;

    while (!isAtEnd()) {
        try {
            processStatement(code);
        } catch (...) {
            synchronize();
        }
    }

    return code;
}

QString CodeGenerator::translateToAssembly() {
    QString data_section = "section .data\n";
    QString bss_section = "section .bss\n";
    QString text_section = "section .text\n    global _start\n\n_start:\n";

    if (symbolTable) {
        for(const auto& sym : symbolTable->getDiscoveredSymbols()) {
            bool isExpression = !sym.value.isEmpty() && (sym.value.contains('+') || sym.value.contains('-') || sym.value.contains('*') || sym.value.contains('/'));
            
            if (!sym.value.isEmpty() && !isExpression) {
                QString val = sym.value;
                if (sym.type == SymbolType::CHAR) {
                    val.remove(0, 1);
                    val.chop(1);
                    data_section += QString("    %1 db '%2'\n").arg(sym.name).arg(val);
                } else {
                    data_section += QString("    %1 dd %2\n").arg(sym.name).arg(val);
                }
            } else {
                if (sym.type == SymbolType::CHAR) {
                    bss_section += QString("    %1 resb 1\n").arg(sym.name);
                } else {
                    bss_section += QString("    %1 resd 1\n").arg(sym.name);
                }
            }
        }
    }

    currentPosition = 0;
    while(!isAtEnd()) {
        Token tok = peek();
        if (tok.getType() == TokenType::KEYWORD && isTypeKeyword(tok.getLexeme())) {
            advance(); 
            QString varName = advance().getLexeme();

            if (match(TokenType::ASSIGN)) {
                Token lhs = advance();
                if(check(TokenType::SEMICOLON)) { 
                    text_section += QString("    ; %1 = %2\n").arg(varName).arg(lhs.getLexeme());
                    QString lhs_operand = lhs.getType() == TokenType::IDENTIFIER ? QString("[%1]").arg(lhs.getLexeme()) : lhs.getLexeme();
                    text_section += QString("    mov eax, %1\n").arg(lhs_operand);
                    text_section += QString("    mov [%1], eax\n\n").arg(varName);
                    match(TokenType::SEMICOLON);
                    continue;
                }
                Token op = advance();
                Token rhs = advance();
                
                QString lhs_operand = lhs.getType() == TokenType::IDENTIFIER ? QString("[%1]").arg(lhs.getLexeme()) : lhs.getLexeme();
                QString rhs_operand = rhs.getType() == TokenType::IDENTIFIER ? QString("[%1]").arg(rhs.getLexeme()) : rhs.getLexeme();

                text_section += QString("    ; %1 = %2 %3 %4\n").arg(varName).arg(lhs.getLexeme()).arg(op.getLexeme()).arg(rhs.getLexeme());
                text_section += QString("    mov eax, %1\n").arg(lhs_operand);

                if (op.getLexeme() == "+") {
                    text_section += QString("    add eax, %1\n").arg(rhs_operand);
                } else if (op.getLexeme() == "-") {
                    text_section += QString("    sub eax, %1\n").arg(rhs_operand);
                }
                text_section += QString("    mov [%1], eax\n\n").arg(varName);
            }
            match(TokenType::SEMICOLON);
        }
        else if (tok.getType() == TokenType::KEYWORD && tok.getLexeme() == "return") {
            advance(); 
            Token retVal = advance(); 
            match(TokenType::SEMICOLON);
            text_section += "    ; return " + retVal.getLexeme() + "\n";
            text_section += "    mov eax, 1\n";
            text_section += "    mov ebx, " + retVal.getLexeme() + "\n";
            text_section += "    int 0x80\n\n";
        }
        else {
            advance(); 
        }
    }

    if (!text_section.contains("int 0x80")) {
        text_section += "    mov eax, 1\n    mov ebx, 0\n    int 0x80\n";
    }

    return data_section + bss_section + text_section;
}

// ============================================================
//  CORE PROCESSING LOGIC
// ============================================================

void CodeGenerator::processStatement(QString& code) {
    if (isAtEnd()) return;

    Token tok = peek();
    QString lexeme = tok.getLexeme();

    // **FIX 1: Handle preprocessor directives**
    if (lexeme.startsWith("#")) {
        processPreprocessor(code);
        return;
    }

    // **FIX 2: Handle 'using namespace'**
    if (lexeme == "using") {
        advance(); // 'using'
        if (peek().getLexeme() == "namespace") {
            advance(); // 'namespace'
            advance(); // identifier (e.g., 'std')
            match(TokenType::SEMICOLON);
        }
        return;
    }

    // **FIX 3: Handle cout**
    if (lexeme == "cout") {
        processCout(code);
        return;
    }

    // **FIX 4: Handle cin**
    if (lexeme == "cin") {
        processCin(code);
        return;
    }

    // **FIX 5: Handle function declarations**
    if (isFunctionDeclaration()) {
        processFunctionDeclaration(code);
        return;
    }

    // Control structures
    if (isControlStructure()) {
        QString kw = lexeme.toLower();
        if (kw == "if") processIfStatement(code);
        else if (kw == "while") processWhileLoop(code);
        else if (kw == "for") processForLoop(code);
        else advance();
        return;
    }

    // **FIX 6: Handle all type keywords properly**
    if (tok.getType() == TokenType::KEYWORD && isTypeKeyword(lexeme)) {
        // Check if it's a function or variable declaration
        if (isFunctionDeclaration()) {
            processFunctionDeclaration(code);
        } else {
            processDeclaration(code);
        }
        return;
    }

    // Other keywords
    if (tok.getType() == TokenType::KEYWORD) {
        QString kw = lexeme.toLower();

        if (kw == "return") {
            code += getIndent() + "return";
            advance();
            if (!check(TokenType::SEMICOLON)) {
                code += " ";
                QString expr;
                processExpression(expr);
                code += expr;
            }
            match(TokenType::SEMICOLON);
            code += "\n";
        } else {
            code += getIndent() + advance().getLexeme() + " ";
        }
        return;
    }

    // Identifiers (assignments or function calls)
    if (tok.getType() == TokenType::IDENTIFIER) {
        processAssignment(code);
        return;
    }

    // Comments
    if (tok.getType() == TokenType::COMMENT) {
        QString comment = advance().getLexeme();
        if (targetLanguage == TargetLanguage::PYTHON) {
            if (comment.startsWith("//")) {
                comment.replace(0, 2, "#");
            } else if (comment.startsWith("/*")) {
                comment = "'''" + comment.mid(2, comment.length() - 4) + "'''";
            }
        }
        code += getIndent() + comment + "\n";
        return;
    }

    // Braces
    if (tok.getType() == TokenType::LBRACE) {
        if (targetLanguage == TargetLanguage::PYTHON) {
            advance();
            indentLevel++;
            processBlock(code);
        } else {
            code += " {\n";
            indentLevel++;
            advance();
            processBlock(code);
        }
        return;
    }

    if (tok.getType() == TokenType::RBRACE) {
        advance();
        if (targetLanguage != TargetLanguage::PYTHON) {
            indentLevel--;
            code += getIndent() + "}\n";
        }
        return;
    }

    // **FIX 7: Handle malformed statements gracefully**
    if (tok.getType() == TokenType::SEMICOLON) {
        advance();
        return;
    }

    // Unknown token - skip it
    code += "# [Skipped unknown token: " + lexeme + "]\n";
    advance();
}

void CodeGenerator::processBlock(QString& code) {
    while (!isAtEnd() && !check(TokenType::RBRACE)) {
        try {
            processStatement(code);
        } catch (...) {
            synchronize();
        }
    }

    if (check(TokenType::RBRACE)) {
        advance();
        if (targetLanguage != TargetLanguage::PYTHON) {
            indentLevel--;
            code += getIndent() + "}\n";
        }
    }
}

// **NEW: Function Declaration Handling**
bool CodeGenerator::isFunctionDeclaration() const {
    if (isAtEnd()) return false;

    // Check pattern: type identifier (
    int pos = currentPosition;

    Token first = tokens[pos];
    if (!isTypeKeyword(first.getLexeme())) return false;

    if (pos + 1 >= tokens.size()) return false;
    Token second = tokens[pos + 1];
    if (second.getType() != TokenType::IDENTIFIER) return false;

    if (pos + 2 >= tokens.size()) return false;
    Token third = tokens[pos + 2];
    return third.getType() == TokenType::LPAREN;
}

void CodeGenerator::processFunctionDeclaration(QString& code) {
    Token returnType = advance();
    Token funcName = advance();

    match(TokenType::LPAREN);

    QStringList params;

    // Parse parameters
    while (!check(TokenType::RPAREN) && !isAtEnd()) {
        if (check(TokenType::KEYWORD)) {
            Token paramType = advance();
            if (check(TokenType::IDENTIFIER)) {
                Token paramName = advance();

                if (targetLanguage == TargetLanguage::PYTHON) {
                    params.append(paramName.getLexeme());
                } else if (targetLanguage == TargetLanguage::JAVA) {
                    params.append(mapType(paramType.getLexeme()) + " " + paramName.getLexeme());
                } else if (targetLanguage == TargetLanguage::JAVASCRIPT) {
                    params.append(paramName.getLexeme());
                } else {
                    params.append(paramType.getLexeme() + " " + paramName.getLexeme());
                }
            }
        }

        if (check(TokenType::COMMA)) {
            advance();
        }
    }

    match(TokenType::RPAREN);

    // Generate function signature
    code += getIndent();

    if (targetLanguage == TargetLanguage::PYTHON) {
        code += QString("def %1(%2):\n").arg(funcName.getLexeme()).arg(params.join(", "));
    } else if (targetLanguage == TargetLanguage::JAVA) {
        QString modifier = (funcName.getLexeme() == "main") ? "public static " : "public static ";
        code += QString("%1%2 %3(%4) {\n")
                    .arg(modifier)
                    .arg(mapType(returnType.getLexeme()))
                    .arg(funcName.getLexeme())
                    .arg(params.join(", "));
    } else if (targetLanguage == TargetLanguage::JAVASCRIPT) {
        code += QString("function %1(%2) {\n").arg(funcName.getLexeme()).arg(params.join(", "));
    } else {
        code += QString("%1 %2(%3) {\n")
        .arg(returnType.getLexeme())
            .arg(funcName.getLexeme())
            .arg(params.join(", "));
    }

    indentLevel++;

    // Process function body
    if (check(TokenType::LBRACE)) {
        advance();
        processBlock(code);
    }

    if (targetLanguage != TargetLanguage::PYTHON) {
        indentLevel--;
        code += getIndent() + "}\n";
    } else {
        indentLevel--;
    }

    code += "\n";
}

// **NEW: cout handling**
void CodeGenerator::processCout(QString& code) {
    advance(); // 'cout'

    code += getIndent();

    if (targetLanguage == TargetLanguage::PYTHON) {
        code += "print(";

        QStringList parts;

        while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
            if (peek().getLexeme() == "<<") {
                advance();
                continue;
            }

            Token t = peek();

            if (t.getLexeme() == "endl") {
                advance();
                // Python's print adds newline by default
                continue;
            }

            if (t.getType() == TokenType::STRING_LITERAL) {
                parts.append(advance().getLexeme());
            } else {
                QString expr;
                while (!check(TokenType::SEMICOLON) && peek().getLexeme() != "<<" && !isAtEnd()) {
                    expr += advance().getLexeme();
                }
                if (!expr.isEmpty()) {
                    parts.append(expr.trimmed());
                }
            }
        }

        code += parts.join(", ") + ")\n";

    } else if (targetLanguage == TargetLanguage::JAVA) {
        code += "System.out.print(";

        QStringList parts;
        bool hasEndl = false;

        while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
            if (peek().getLexeme() == "<<") {
                advance();
                continue;
            }

            Token t = peek();

            if (t.getLexeme() == "endl") {
                hasEndl = true;
                advance();
                continue;
            }

            QString expr;
            while (!check(TokenType::SEMICOLON) && peek().getLexeme() != "<<" && !isAtEnd()) {
                expr += advance().getLexeme() + " ";
            }
            if (!expr.isEmpty()) {
                parts.append(expr.trimmed());
            }
        }

        if (hasEndl) {
            code.replace("print(", "println(");
        }

        code += parts.join(" + ") + ");\n";

    } else if (targetLanguage == TargetLanguage::JAVASCRIPT) {
        code += "console.log(";

        QStringList parts;

        while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
            if (peek().getLexeme() == "<<") {
                advance();
                continue;
            }

            Token t = peek();

            if (t.getLexeme() == "endl") {
                advance();
                continue;
            }

            QString expr;
            while (!check(TokenType::SEMICOLON) && peek().getLexeme() != "<<" && !isAtEnd()) {
                expr += advance().getLexeme() + " ";
            }
            if (!expr.isEmpty()) {
                parts.append(expr.trimmed());
            }
        }

        code += parts.join(" + ") + ");\n";
    }

    match(TokenType::SEMICOLON);
}

// **NEW: cin handling**
void CodeGenerator::processCin(QString& code) {
    advance(); // 'cin'

    QStringList vars;

    while (!check(TokenType::SEMICOLON) && !isAtEnd()) {
        if (peek().getLexeme() == ">>") {
            advance();
            continue;
        }

        if (check(TokenType::IDENTIFIER)) {
            vars.append(advance().getLexeme());
        } else {
            advance();
        }
    }

    match(TokenType::SEMICOLON);

    for (const QString& var : vars) {
        code += getIndent();

        if (targetLanguage == TargetLanguage::PYTHON) {
            code += QString("%1 = input()\n").arg(var);
        } else if (targetLanguage == TargetLanguage::JAVA) {
            code += QString("Scanner scanner = new Scanner(System.in);\n");
            code += getIndent();
            code += QString("%1 = scanner.nextLine();\n").arg(var);
        } else if (targetLanguage == TargetLanguage::JAVASCRIPT) {
            code += QString("// %1 = readline() // (Node.js requires 'readline' module)\n").arg(var);
        }
    }
}

// **NEW: Preprocessor handling**
void CodeGenerator::processPreprocessor(QString& code) {
    QString directive = advance().getLexeme(); // #include
    QString header;

    if(check(TokenType::LESS_THAN)) {
        advance(); // <
        while(!isAtEnd() && !check(TokenType::GREATER_THAN)) {
            header += advance().getLexeme();
        }
        advance(); // >
    }

    if (header == "iostream") {
        if (targetLanguage == TargetLanguage::JAVA) {
            code += getIndent() + "// C++ <iostream> is handled by System.out and java.util.Scanner\n";
        } else if (targetLanguage == TargetLanguage::PYTHON) {
            code += getIndent() + "# C++ <iostream> is equivalent to standard input/output functions like print() and input()\n";
        } else if (targetLanguage == TargetLanguage::JAVASCRIPT) {
            code += getIndent() + "// C++ <iostream> is equivalent to console.log() and prompt() or process.stdin\n";
        }
    } else if (header == "string") {
         if (targetLanguage == TargetLanguage::JAVA) {
            code += getIndent() + "// C++ <string> corresponds to the built-in String class\n";
        } else if (targetLanguage == TargetLanguage::PYTHON || targetLanguage == TargetLanguage::JAVASCRIPT) {
            code += getIndent() + "# C++ <string> corresponds to the built-in string type\n";
        }
    }

    // Skip to end of line
    while (!isAtEnd() && peek().getLexeme() != "\n") {
        if (check(TokenType::KEYWORD) || check(TokenType::IDENTIFIER)) break;
        advance();
    }
}

// --- CONTROL STRUCTURES ---

void CodeGenerator::processIfStatement(QString& code) {
    advance(); // 'if'

    code += getIndent();
    if (targetLanguage == TargetLanguage::PYTHON) code += "if ";
    else code += "if (";

    QString cond;
    processCondition(cond);
    code += cond;

    if (targetLanguage == TargetLanguage::PYTHON) code += ":\n";
    else code += ") {\n";

    indentLevel++;

    if (check(TokenType::LBRACE)) {
        advance();
        processBlock(code);
        indentLevel--;
        // processBlock handles closing brace
    } else {
        processStatement(code);
        indentLevel--;
        if (targetLanguage != TargetLanguage::PYTHON) code += getIndent() + "}\n";
    }

    // **FIX 8: Handle 'else' without 'if'**
    if (check(TokenType::KEYWORD) && peek().getLexeme() == "else") {
        advance();
        code += getIndent();

        if (check(TokenType::KEYWORD) && peek().getLexeme() == "if") {
            if (targetLanguage == TargetLanguage::PYTHON) code += "el";
            else code += "else ";
            processIfStatement(code);
        } else {
            if (targetLanguage == TargetLanguage::PYTHON) code += "else:\n";
            else code += "else {\n";

            indentLevel++;
            if (check(TokenType::LBRACE)) {
                advance();
                processBlock(code);
                indentLevel--;
            } else {
                processStatement(code);
                indentLevel--;
                if (targetLanguage != TargetLanguage::PYTHON) code += getIndent() + "}\n";
            }
        }
    }
}

void CodeGenerator::processWhileLoop(QString& code) {
    advance();
    code += getIndent();

    if (targetLanguage == TargetLanguage::PYTHON) code += "while ";
    else code += "while (";

    QString cond;
    processCondition(cond);
    code += cond;

    if (targetLanguage == TargetLanguage::PYTHON) code += ":\n";
    else code += ") {\n";

    indentLevel++;
    if (check(TokenType::LBRACE)) {
        advance();
        processBlock(code);
        indentLevel--;
    } else {
        processStatement(code);
        indentLevel--;
        if (targetLanguage != TargetLanguage::PYTHON) code += getIndent() + "}\n";
    }
}

void CodeGenerator::processForLoop(QString& code) {
    advance();
    match(TokenType::LPAREN);

    QString init, cond, inc;

    while(!check(TokenType::SEMICOLON) && !isAtEnd()) {
        init += advance().getLexeme() + " ";
    }
    match(TokenType::SEMICOLON);

    while(!check(TokenType::SEMICOLON) && !isAtEnd()) {
        cond += advance().getLexeme() + " ";
    }
    match(TokenType::SEMICOLON);

    while(!check(TokenType::RPAREN) && !check(TokenType::LBRACE) && !isAtEnd()) {
        inc += advance().getLexeme() + " ";
    }
    match(TokenType::RPAREN);

    code += getIndent();

    if (targetLanguage == TargetLanguage::PYTHON) {
        QString loopVar = extractLoopVariable(init);
        QString rangeParams = convertConditionToRange(cond, init);
        code += QString("for %1 in %2:\n").arg(loopVar).arg(rangeParams);
    } else {
        code += QString("for (%1; %2; %3) {\n").arg(init.trimmed()).arg(cond.trimmed()).arg(inc.trimmed());
    }

    indentLevel++;
    if (check(TokenType::LBRACE)) {
        advance();
        processBlock(code);
        indentLevel--; 
    } else {
        processStatement(code);
        indentLevel--;
        if (targetLanguage != TargetLanguage::PYTHON) code += getIndent() + "}\n";
    }
}

// --- BASIC STATEMENTS ---

void CodeGenerator::processDeclaration(QString& code) {
    Token typeTok = advance();

    // **FIX 9: Handle missing identifier**
    if (!check(TokenType::IDENTIFIER)) {
        code += getIndent() + "# Error: Missing identifier after type '" + typeTok.getLexeme() + "'\n";
        skipToNextStatement();
        return;
    }

    Token idTok = advance();

    code += getIndent();

    if (targetLanguage == TargetLanguage::JAVASCRIPT) {
        code += "let " + idTok.getLexeme();
    } else if (targetLanguage == TargetLanguage::PYTHON) {
        code += idTok.getLexeme();
    } else if (targetLanguage == TargetLanguage::JAVA) {
        code += mapType(typeTok.getLexeme()) + " " + idTok.getLexeme();
    } else {
        code += typeTok.getLexeme() + " " + idTok.getLexeme();
    }

    if (match(TokenType::ASSIGN)) {
        code += " = ";
        QString expr;
        processExpression(expr);
        code += expr;
    } else {
        // **FIX 10: Default initialization for Python**
        if (targetLanguage == TargetLanguage::PYTHON) {
            code += " = None";
        }
    }

    if (targetLanguage != TargetLanguage::PYTHON) code += ";\n";
    else code += "\n";

    match(TokenType::SEMICOLON);
}

void CodeGenerator::processAssignment(QString& code) {
    Token idTok = advance();

    code += getIndent() + idTok.getLexeme();

    // Check for function call
    if (check(TokenType::LPAREN)) {
        advance(); // (

        QStringList args;
        while (!check(TokenType::RPAREN) && !isAtEnd()) {
            QString arg;
            int parenDepth = 0;

            while (!isAtEnd()) {
                if (check(TokenType::LPAREN)) parenDepth++;
                if (check(TokenType::RPAREN)) {
                    if (parenDepth == 0) break;
                    parenDepth--;
                }
                if (check(TokenType::COMMA) && parenDepth == 0) break;

                arg += advance().getLexeme() + " ";
            }

            if (!arg.trimmed().isEmpty()) {
                args.append(arg.trimmed());
            }

            if (check(TokenType::COMMA)) advance();
        }

        match(TokenType::RPAREN);

        code += "(" + args.join(", ") + ")";

        if (targetLanguage != TargetLanguage::PYTHON) code += ";\n";
        else code += "\n";

        match(TokenType::SEMICOLON);
        return;
    }

    if (match(TokenType::ASSIGN)) {
        code += " = ";
        QString expr;
        processExpression(expr);
        code += expr;
    }

    if (targetLanguage != TargetLanguage::PYTHON) code += ";\n";
    else code += "\n";

    match(TokenType::SEMICOLON);
}

void CodeGenerator::processExpression(QString& result) {
    while (!isAtEnd() && !check(TokenType::SEMICOLON) && !check(TokenType::RPAREN) && !check(TokenType::COMMA)) {
        Token t = advance();
        QString lex = t.getLexeme();

        // **FIX 11: Handle NOT operator (!)**
        if (lex == "!") {
            if (targetLanguage == TargetLanguage::PYTHON) {
                lex = "not ";
            }
        }
        // Boolean conversions
        else if (targetLanguage == TargetLanguage::PYTHON) {
            if (lex == "true") lex = "True";
            else if (lex == "false") lex = "False";
            else if (lex == "NULL" || lex == "nullptr") lex = "None";
            else if (lex == "&&") lex = "and";
            else if (lex == "||") lex = "or";
        }

        result += lex + " ";
    }
    result = result.trimmed();
}

void CodeGenerator::processCondition(QString& result) {
    match(TokenType::LPAREN);

    int parenDepth = 0;

    while (!isAtEnd()) {
        if (check(TokenType::LPAREN)) parenDepth++;
        if (check(TokenType::RPAREN)) {
            if (parenDepth == 0) break;
            parenDepth--;
        }
        if (check(TokenType::LBRACE) && parenDepth == 0) break;

        Token t = advance();
        QString lex = t.getLexeme();

        // **FIX 12: Handle all boolean operators in conditions**
        if (lex == "!") {
            if (targetLanguage == TargetLanguage::PYTHON) {
                lex = "not ";
            }
        } else if (targetLanguage == TargetLanguage::PYTHON) {
            if (lex == "true") lex = "True";
            else if (lex == "false") lex = "False";
            else if (lex == "&&") lex = "and";
            else if (lex == "||") lex = "or";
        }

        result += lex + " ";
    }

    match(TokenType::RPAREN);
    result = result.trimmed();
}

// --- HELPER UTILITIES ---

QString CodeGenerator::getIndent() const {
    return QString(indentLevel * 4, ' ');
}

QString CodeGenerator::mapType(const QString& cppType) {
    QString lower = cppType.toLower();

    if (targetLanguage == TargetLanguage::JAVA) {
        if (lower == "string") return "String";
        if (lower == "bool") return "boolean";
    }

    return cppType;
}

bool CodeGenerator::isTypeKeyword(const QString& keyword) const {
    QString lower = keyword.toLower();
    return lower == "int" || lower == "float" || lower == "string" ||
           lower == "bool" || lower == "char" || lower == "double" ||
           lower == "short" || lower == "long" || lower == "void";
}

QString CodeGenerator::extractLoopVariable(const QString& init) {
    int eqIndex = init.indexOf('=');
    if (eqIndex == -1) return "i";

    QString leftSide = init.left(eqIndex).trimmed();
    QStringList parts = leftSide.split(' ');
    return parts.last();
}

QString CodeGenerator::convertConditionToRange(const QString& condition, const QString& init) {
    QString limit = "10";

    if (condition.contains("<=")) {
        QStringList parts = condition.split("<=");
        if (parts.size() > 1) {
            QString rawLimit = parts.last().trimmed();
            bool ok;
            int val = rawLimit.toInt(&ok);
            if (ok) {
                limit = QString::number(val + 1);
            } else {
                limit = rawLimit + " + 1";
            }
        }
    }
    else if (condition.contains("<")) {
        limit = condition.split('<').last().trimmed();
    }

    QString start = "0";
    if (init.contains("=")) {
        start = init.split('=').last().trimmed();
    }

    return QString("range(%1, %2)").arg(start).arg(limit);
}

QString CodeGenerator::generateLabel() {
    return QString("L%1").arg(labelCounter++);
}

// **NEW: Error recovery**
void CodeGenerator::skipToNextStatement() {
    while (!isAtEnd() && !check(TokenType::SEMICOLON) && !check(TokenType::LBRACE) && !check(TokenType::RBRACE)) {
        advance();
    }
    if (check(TokenType::SEMICOLON)) advance();
}

void CodeGenerator::synchronize() {
    skipToNextStatement();
}

// --- TOKEN NAVIGATION ---

Token CodeGenerator::peek() const {
    if (currentPosition >= tokens.size()) return Token(TokenType::END_OF_FILE, "", 0, 0);
    return tokens[currentPosition];
}

Token CodeGenerator::peekAhead(int offset) const {
    int pos = currentPosition + offset;
    if (pos >= tokens.size()) return Token(TokenType::END_OF_FILE, "", 0, 0);
    return tokens[pos];
}

Token CodeGenerator::advance() {
    if (currentPosition < tokens.size()) return tokens[currentPosition++];
    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

bool CodeGenerator::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool CodeGenerator::check(TokenType type) const {
    if (isAtEnd()) return false;
    return tokens[currentPosition].getType() == type;
}

bool CodeGenerator::isAtEnd() const {
    return currentPosition >= tokens.size();
}

bool CodeGenerator::isControlStructure() const {
    if (isAtEnd()) return false;
    QString lex = tokens[currentPosition].getLexeme().toLower();
    return lex == "if" || lex == "while" || lex == "for";
}
