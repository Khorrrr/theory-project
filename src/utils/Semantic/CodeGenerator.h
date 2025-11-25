#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "./models/LexicalAnalysis/Token.h"
#include "./models/Semantic/SymbolTable.h"
#include <QString>
#include <QVector>
#include <QProcess>

enum class TargetLanguage {
    PYTHON,
    JAVA,
    JAVASCRIPT,
    ASSEMBLY
};

class CodeGenerator {
private:
    QVector<Token> tokens;
    SymbolTable* symbolTable;
    TargetLanguage targetLanguage;
    QString generatedCode;
    QString m_sourceCode;

    // State variables
    int indentLevel;
    int currentPosition;
    int labelCounter;
    bool inGlobalScope; // NEW: Track if we're in global scope

public:
    CodeGenerator();
    ~CodeGenerator();

    void setTokens(const QVector<Token>& toks);
    void setSymbolTable(SymbolTable* table);
    void setTargetLanguage(TargetLanguage lang);

    void setSourceCode(const QString& source);

    QString generate();
    QString getGeneratedCode() const { return generatedCode; }
    void reset();

private:
    // --- Generation Strategies ---
    QString translateToPython();
    QString translateToJava();
    QString translateToJavaScript();
    QString translateToAssembly();



    // --- Optimization ---
    void optimizeTokens();

    // --- Core Processing Logic ---
    void processStatement(QString& code);
    void processBlock(QString& code);

    // NEW: Function handling
    void processFunctionDeclaration(QString& code);
    bool isFunctionDeclaration() const;

    // Control Structures
    void processIfStatement(QString& code);
    void processWhileLoop(QString& code);
    void processForLoop(QString& code);

    // Basic Statements
    void processDeclaration(QString& code);
    void processAssignment(QString& code);
    void processExpression(QString& result);
    void processCondition(QString& result);

    // NEW: Stream operations
    void processCout(QString& code);
    void processCin(QString& code);

    // NEW: Preprocessor
    void processPreprocessor(QString& code);

    // --- Helpers ---
    QString getIndent() const;
    QString extractLoopVariable(const QString& init);
    QString convertConditionToRange(const QString& condition, const QString& init);
    QString generateLabel();
    QString mapType(const QString& cppType); // NEW: Type mapping
    bool isTypeKeyword(const QString& keyword) const; // NEW

    // --- Token Navigation ---
    Token peek() const;
    Token peekAhead(int offset) const; // NEW: Look ahead multiple tokens
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    bool isAtEnd() const;
    bool isControlStructure() const;

    // NEW: Error recovery
    void skipToNextStatement();
    void synchronize();
};

#endif // CODEGENERATOR_H
