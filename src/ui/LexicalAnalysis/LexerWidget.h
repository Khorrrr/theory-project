#ifndef LEXERWIDGET_H
#define LEXERWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include "src/utils/LexicalAnalysis/Lexer.h"
#include "src/utils/LexicalAnalysis/AutomatonManager.h"

class LexerWidget : public QWidget {
    Q_OBJECT

private:
    QTextEdit* inputTextEdit;
    QTableWidget* tokensTable;
    QTextEdit* errorTextEdit;
    QPushButton* tokenizeButton;
    QPushButton* clearButton;
    QCheckBox* skipWhitespaceCheckBox;
    QCheckBox* skipCommentsCheckBox;
    QLabel* statusLabel;

    Lexer* lexer;
    AutomatonManager* automatonManager;

public:
    explicit LexerWidget(QWidget *parent = nullptr);
    ~LexerWidget();

    void setAutomatonManager(AutomatonManager* manager);

private slots:
    void onTokenizeClicked();
    void onClearClicked();

private:
    void setupUI();
    void createConnections();
    void displayTokens(const QVector<Token>& tokens);
    void displayErrors(const QVector<LexerError>& errors);
};

#endif // LEXERWIDGET_H
