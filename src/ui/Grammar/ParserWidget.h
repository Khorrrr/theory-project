#ifndef PARSERWIDGET_H
#define PARSERWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include "./src/models/Grammar/Grammar.h"
#include "./src/utils/Grammar/Parser.h"
#include "ParseTreeWidget.h"
#include "./src/utils/LexicalAnalysis/Lexer.h"
#include "./src/utils/LexicalAnalysis/AutomatonManager.h"

class ParserWidget : public QWidget {
    Q_OBJECT

private:
    // UI Components
    QComboBox* grammarSelector;
    QPushButton* loadGrammarBtn;
    QPushButton* addProductionBtn;
    QPushButton* clearGrammarBtn;

    QListWidget* productionsList;
    QLineEdit* productionInput;

    QTextEdit* inputTextEdit;
    QPushButton* parseButton;
    QPushButton* clearButton;

    QTextEdit* grammarInfoText;
    QTextEdit* parseOutputText;

    ParseTreeWidget* parseTreeWidget;

    QLabel* statusLabel;

    // Core components
    Grammar* currentGrammar;
    Parser* parser;
    Lexer* lexer;
    AutomatonManager* automatonManager;

public:
    explicit ParserWidget(QWidget *parent = nullptr);
    ~ParserWidget();

    void setAutomatonManager(AutomatonManager* manager);

private slots:
    void onLoadGrammar();
    void onAddProduction();
    void onClearGrammar();
    void onParseClicked();
    void onClearClicked();
    void onProductionSelected(int index);
    void onDeleteProduction();

private:
    void setupUI();
    void createConnections();
    void updateGrammarDisplay();
    void updateProductionsList();
    void displayParseResult(const ParseTree& tree);
    void displayParseErrors(const QVector<ParseError>& errors);

    void loadPredefinedGrammar(const QString& grammarName);
};

#endif // PARSERWIDGET_H
