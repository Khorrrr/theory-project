#ifndef SEMANTICANALYZERWIDGET_H
#define SEMANTICANALYZERWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "./src/utils/Semantic/SemanticAnalyzer.h"
#include "./src/utils/Semantic/CodeGenerator.h"
#include "./src/utils/LexicalAnalysis/Lexer.h"
#include "./src/utils/LexicalAnalysis/AutomatonManager.h"

#include <QCheckBox>

class SemanticAnalyzerWidget : public QWidget {
    Q_OBJECT

private:
    // UI Components
    QTextEdit* sourceCodeEdit;
    QPushButton* analyzeButton;
    QPushButton* translateButton;
    QPushButton* clearButton;

    QComboBox* targetLanguageCombo;


    QTableWidget* symbolTableWidget;
    QTextEdit* errorsWarningsText;
    QTextEdit* translatedCodeEdit;

    QLabel* statusLabel;

    // Core components
    SemanticAnalyzer* semanticAnalyzer;
    CodeGenerator* codeGenerator;
    Lexer* lexer;
    AutomatonManager* automatonManager;

public:
    explicit SemanticAnalyzerWidget(QWidget *parent = nullptr);
    ~SemanticAnalyzerWidget();

    void setAutomatonManager(AutomatonManager* manager);

private slots:
    void onAnalyzeClicked();
    void onTranslateClicked();
    void onClearClicked();


private:
    void setupUI();
    void createConnections();
    void displaySymbolTable();
    void displayErrorsWarnings();
    void displayTranslatedCode(const QString& code);
};

#endif // SEMANTICANALYZERWIDGET_H
