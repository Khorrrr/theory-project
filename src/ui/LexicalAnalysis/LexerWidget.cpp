#include "LexerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>

LexerWidget::LexerWidget(QWidget *parent)
    : QWidget(parent), automatonManager(nullptr) {

    lexer = new Lexer();
    setupUI();
    createConnections();
}

LexerWidget::~LexerWidget() {
    delete lexer;
}

void LexerWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void LexerWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Lexical Analyzer");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QSplitter* splitter = new QSplitter(Qt::Vertical);

    QGroupBox* inputGroup = new QGroupBox("Source Code Input");
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);

    inputTextEdit = new QTextEdit();
    inputTextEdit->setPlaceholderText("Enter your source code here...");
    inputTextEdit->setMinimumHeight(150);
    inputTextEdit->setText(
        "int main() {\n"
        "    int x = 10;\n"
        "    float y = 3.14;\n"
        "    if (x > 5) {\n"
        "        return x + y;\n"
        "    }\n"
        "    return 0;\n"
        "}"
        );
    inputLayout->addWidget(inputTextEdit);

    QHBoxLayout* optionsLayout = new QHBoxLayout();
    skipWhitespaceCheckBox = new QCheckBox("Skip Whitespace");
    skipWhitespaceCheckBox->setChecked(true);
    skipCommentsCheckBox = new QCheckBox("Skip Comments");
    skipCommentsCheckBox->setChecked(true);

    optionsLayout->addWidget(skipWhitespaceCheckBox);
    optionsLayout->addWidget(skipCommentsCheckBox);
    optionsLayout->addStretch();

    inputLayout->addLayout(optionsLayout);
    splitter->addWidget(inputGroup);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    tokenizeButton = new QPushButton("🔍 Tokenize");
    tokenizeButton->setStyleSheet("QPushButton { padding: 8px 20px; font-size: 12pt; }");
    clearButton = new QPushButton("🗑️ Clear");
    clearButton->setStyleSheet("QPushButton { padding: 8px 20px; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(tokenizeButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    QGroupBox* tokensGroup = new QGroupBox("Tokens");
    QVBoxLayout* tokensLayout = new QVBoxLayout(tokensGroup);

    tokensTable = new QTableWidget();
    tokensTable->setColumnCount(5);
    tokensTable->setHorizontalHeaderLabels({"#", "Type", "Lexeme", "Line", "Column"});

    // ===== BLACK BACKGROUND WITH WHITE TEXT STYLING =====
    tokensTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #1e1e1e;"  // Dark background
        "   color: white;"                 // White text
        "   gridline-color: #3e3e3e;"     // Dark grid lines
        "   border: 1px solid #555;"
        "}"
        "QTableWidget::item {"
        "   color: white;"                 // White text for items
        "   padding: 5px;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #0078d7;"   // Blue selection
        "   color: white;"
        "}"
        "QHeaderView::section {"
        "   background-color: #2d2d2d;"   // Dark header background
        "   color: white;"                 // White header text
        "   padding: 5px;"
        "   border: 1px solid #3e3e3e;"
        "   font-weight: bold;"
        "}"
        );

    tokensTable->horizontalHeader()->setStretchLastSection(false);
    tokensTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tokensTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tokensTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tokensTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tokensTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tokensTable->setAlternatingRowColors(true);
    tokensTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokensTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tokensTable->setMinimumHeight(200);

    tokensLayout->addWidget(tokensTable);
    splitter->addWidget(tokensGroup);

    QGroupBox* errorsGroup = new QGroupBox("Errors and Warnings");
    QVBoxLayout* errorsLayout = new QVBoxLayout(errorsGroup);

    errorTextEdit = new QTextEdit();
    errorTextEdit->setReadOnly(true);
    errorTextEdit->setMaximumHeight(100);
    errorTextEdit->setStyleSheet("QTextEdit { background-color: #fff3cd; color: #856404; }");

    errorsLayout->addWidget(errorTextEdit);
    splitter->addWidget(errorsGroup);

    mainLayout->addWidget(splitter);

    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);
}

void LexerWidget::createConnections() {
    connect(tokenizeButton, &QPushButton::clicked, this, &LexerWidget::onTokenizeClicked);
    connect(clearButton, &QPushButton::clicked, this, &LexerWidget::onClearClicked);
}

void LexerWidget::onTokenizeClicked() {
    QString sourceCode = inputTextEdit->toPlainText();

    if (sourceCode.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Empty Input", "Please enter some source code to tokenize.");
        return;
    }

    if (!lexer) {
        QMessageBox::critical(this, "Error", "Lexer not initialized!");
        return;
    }

    lexer->setSkipWhitespace(skipWhitespaceCheckBox->isChecked());
    lexer->setSkipComments(skipCommentsCheckBox->isChecked());

    bool success = lexer->tokenize(sourceCode);

    displayTokens(lexer->getTokens());
    displayErrors(lexer->getErrors());

    if (success) {
        statusLabel->setText(QString("✅ Tokenization successful! Generated %1 tokens.")
                                 .arg(lexer->getTokens().size()));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d4edda; color: #155724; border-radius: 3px; }");
    } else {
        statusLabel->setText(QString("❌ Tokenization completed with %1 error(s).")
                                 .arg(lexer->getErrors().size()));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f8d7da; color: #721c24; border-radius: 3px; }");
    }
}

void LexerWidget::onClearClicked() {
    inputTextEdit->clear();
    tokensTable->setRowCount(0);
    errorTextEdit->clear();
    statusLabel->setText("Ready");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    if (lexer) {
        lexer->reset();
    }
}

void LexerWidget::displayTokens(const QVector<Token>& tokens) {
    tokensTable->setRowCount(0);

    int row = 0;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::END_OF_FILE) continue;

        tokensTable->insertRow(row);

        // Create table items
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(row + 1));
        QTableWidgetItem* typeItem = new QTableWidgetItem(token.getTypeString());
        QTableWidgetItem* lexemeItem = new QTableWidgetItem(token.getLexeme());
        QTableWidgetItem* lineItem = new QTableWidgetItem(QString::number(token.getLine()));
        QTableWidgetItem* colItem = new QTableWidgetItem(QString::number(token.getColumn()));

        // Set text color to white for all items
        numItem->setForeground(Qt::white);
        typeItem->setForeground(Qt::white);
        lexemeItem->setForeground(Qt::white);
        lineItem->setForeground(Qt::white);
        colItem->setForeground(Qt::white);

        // Set background colors based on token type (darker shades for black background)
        QColor bgColor;
        switch (token.getType()) {
        case TokenType::KEYWORD:
            bgColor = QColor(30, 100, 150);      // Dark blue
            break;
        case TokenType::IDENTIFIER:
            bgColor = QColor(150, 120, 50);      // Dark gold
            break;
        case TokenType::INTEGER_LITERAL:
        case TokenType::FLOAT_LITERAL:
            bgColor = QColor(50, 150, 50);       // Dark green
            break;
        case TokenType::STRING_LITERAL:
        case TokenType::CHAR_LITERAL:
            bgColor = QColor(150, 50, 100);      // Dark pink/purple
            break;
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::MULTIPLY:
        case TokenType::DIVIDE:
        case TokenType::MODULO:
        case TokenType::ASSIGN:
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::LESS_THAN:
        case TokenType::GREATER_THAN:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
        case TokenType::LOGICAL_AND:
        case TokenType::LOGICAL_OR:
        case TokenType::LOGICAL_NOT:
            bgColor = QColor(120, 80, 50);       // Dark orange
            break;
        case TokenType::SEMICOLON:
        case TokenType::COMMA:
        case TokenType::DOT:
        case TokenType::COLON:
        case TokenType::LPAREN:
        case TokenType::RPAREN:
        case TokenType::LBRACE:
        case TokenType::RBRACE:
        case TokenType::LBRACKET:
        case TokenType::RBRACKET:
            bgColor = QColor(80, 80, 120);       // Dark purple
            break;
        default:
            bgColor = QColor(50, 50, 50);        // Very dark gray
            break;
        }

        numItem->setBackground(bgColor);
        typeItem->setBackground(bgColor);
        lexemeItem->setBackground(bgColor);
        lineItem->setBackground(bgColor);
        colItem->setBackground(bgColor);

        // Add items to table
        tokensTable->setItem(row, 0, numItem);
        tokensTable->setItem(row, 1, typeItem);
        tokensTable->setItem(row, 2, lexemeItem);
        tokensTable->setItem(row, 3, lineItem);
        tokensTable->setItem(row, 4, colItem);

        row++;
    }
}

void LexerWidget::displayErrors(const QVector<LexerError>& errors) {
    errorTextEdit->clear();

    if (errors.isEmpty()) {
        errorTextEdit->setText("✅ No errors found!");
        errorTextEdit->setStyleSheet("QTextEdit { background-color: #d4edda; color: #155724; }");
    } else {
        QString errorText;
        for (const auto& error : errors) {
            errorText += "❌ " + error.toString() + "\n";
        }
        errorTextEdit->setText(errorText);
        errorTextEdit->setStyleSheet("QTextEdit { background-color: #f8d7da; color: #721c24; }");
    }
}
