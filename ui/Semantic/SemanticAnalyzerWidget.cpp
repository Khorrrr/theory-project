#include "SemanticAnalyzerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>

SemanticAnalyzerWidget::SemanticAnalyzerWidget(QWidget *parent)
    : QWidget(parent), automatonManager(nullptr) {

    semanticAnalyzer = new SemanticAnalyzer();
    codeGenerator = new CodeGenerator();
    lexer = new Lexer();
    mlBridge = new MLTranslationBridge(this);

    setupUI();
    createConnections();
}

SemanticAnalyzerWidget::~SemanticAnalyzerWidget() {
    delete semanticAnalyzer;
    delete codeGenerator;
    delete lexer;
    delete mlBridge;
}

void SemanticAnalyzerWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void SemanticAnalyzerWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Semantic Analyzer & Code Translator");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Main splitter
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);

    // ===== LEFT PANEL: Source Code =====
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    QGroupBox* sourceGroup = new QGroupBox("Source Code");
    QVBoxLayout* sourceLayout = new QVBoxLayout();

    sourceCodeEdit = new QTextEdit();
    sourceCodeEdit->setPlaceholderText("Enter your source code here...");
    sourceCodeEdit->setText(
        "int x = 10;\n"
        "float y = 3.14;\n"
        "int z = x + 5;\n"
        "char c = 'A';\n"
        "bool flag = true;"
        );
    sourceLayout->addWidget(sourceCodeEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    analyzeButton = new QPushButton("🔍 Analyze");
    analyzeButton->setStyleSheet(
        "QPushButton { padding: 8px 20px; font-size: 11pt; font-weight: bold; "
        "background-color: #0078d7; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #005a9e; }"
        );

    translateButton = new QPushButton("🔄 Translate");
    translateButton->setStyleSheet(
        "QPushButton { padding: 8px 20px; font-size: 11pt; font-weight: bold; "
        "background-color: #28a745; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #218838; }"
        );
    translateButton->setEnabled(false);

    clearButton = new QPushButton("🗑️ Clear");
    clearButton->setStyleSheet("QPushButton { padding: 8px 20px; }");

    buttonLayout->addWidget(analyzeButton);
    buttonLayout->addWidget(translateButton);
    buttonLayout->addWidget(clearButton);
    sourceLayout->addLayout(buttonLayout);

    sourceGroup->setLayout(sourceLayout);
    leftLayout->addWidget(sourceGroup);

    // Symbol Table
    QGroupBox* symbolGroup = new QGroupBox("Symbol Table");
    QVBoxLayout* symbolLayout = new QVBoxLayout();

    symbolTableWidget = new QTableWidget();
    symbolTableWidget->setColumnCount(5);
    symbolTableWidget->setHorizontalHeaderLabels({"Name", "Type", "Value", "Scope", "Status"});
    symbolTableWidget->setStyleSheet(
        "QTableWidget { background-color: #1e1e1e; color: white; gridline-color: #3e3e3e; }"
        "QTableWidget::item { color: white; padding: 5px; }"
        "QTableWidget::item:selected { background-color: #0078d7; }"
        "QHeaderView::section { background-color: #2d2d2d; color: white; padding: 5px; "
        "border: 1px solid #3e3e3e; font-weight: bold; }"
        );
    symbolTableWidget->horizontalHeader()->setStretchLastSection(true);
    symbolTableWidget->setAlternatingRowColors(true);
    symbolTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    symbolLayout->addWidget(symbolTableWidget);

    symbolGroup->setLayout(symbolLayout);
    leftLayout->addWidget(symbolGroup);

    mainSplitter->addWidget(leftPanel);

    // ===== RIGHT PANEL: Analysis Results =====
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    // Errors/Warnings
    QGroupBox* errorsGroup = new QGroupBox("Errors & Warnings");
    QVBoxLayout* errorsLayout = new QVBoxLayout();

    errorsWarningsText = new QTextEdit();
    errorsWarningsText->setReadOnly(true);
    errorsWarningsText->setMaximumHeight(150);
    errorsWarningsText->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: white; }");
    errorsLayout->addWidget(errorsWarningsText);

    errorsGroup->setLayout(errorsLayout);
    rightLayout->addWidget(errorsGroup);

    // Translation settings
    QGroupBox* translateGroup = new QGroupBox("Code Translation");
    QVBoxLayout* translateLayout = new QVBoxLayout();

    // Translation method selection
    translationMethodGroup = new QGroupBox("Translation Method");
    QVBoxLayout* methodLayout = new QVBoxLayout();
    QHBoxLayout* radioLayout = new QHBoxLayout();

    ruleBasedRadio = new QRadioButton("Rule-Based Generator");
    mlBasedRadio = new QRadioButton("ML Translation");

    // Set default selection
    ruleBasedRadio->setChecked(true);

    // Style radio buttons to match existing UI
    QString radioStyle = "QRadioButton { font-size: 10pt; padding: 3px; }"
                         "QRadioButton::indicator { width: 12px; height: 12px; }";
    ruleBasedRadio->setStyleSheet(radioStyle);
    mlBasedRadio->setStyleSheet(radioStyle);

    radioLayout->addWidget(ruleBasedRadio);
    radioLayout->addWidget(mlBasedRadio);
    radioLayout->addStretch();
    methodLayout->addLayout(radioLayout);
    translationMethodGroup->setLayout(methodLayout);
    translateLayout->addWidget(translationMethodGroup);

    // Target language selection
    QHBoxLayout* langLayout = new QHBoxLayout();
    langLayout->addWidget(new QLabel("Target Language:"));

    targetLanguageCombo = new QComboBox();
    targetLanguageCombo->addItem("Python", QVariant::fromValue(TargetLanguage::PYTHON));
    targetLanguageCombo->addItem("Java", QVariant::fromValue(TargetLanguage::JAVA));
    targetLanguageCombo->addItem("JavaScript", QVariant::fromValue(TargetLanguage::JAVASCRIPT));
    targetLanguageCombo->addItem("Assembly", QVariant::fromValue(TargetLanguage::ASSEMBLY));
    langLayout->addWidget(targetLanguageCombo);
    langLayout->addStretch();

    translateLayout->addLayout(langLayout);



    translatedCodeEdit = new QTextEdit();
    translatedCodeEdit->setReadOnly(true);
    translatedCodeEdit->setStyleSheet(
        "QTextEdit { background-color: #1e1e1e; color: #4ec9b0; "
        "font-family: 'Courier New', monospace; font-size: 10pt; }"
        );
    translateLayout->addWidget(translatedCodeEdit);

    translateGroup->setLayout(translateLayout);
    rightLayout->addWidget(translateGroup);

    mainSplitter->addWidget(rightPanel);
    mainLayout->addWidget(mainSplitter);

    // Status
    statusLabel = new QLabel("Ready - Enter source code and click Analyze");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);
}

void SemanticAnalyzerWidget::createConnections() {
    connect(analyzeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onAnalyzeClicked);
    connect(translateButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onTranslateClicked);
    connect(clearButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onClearClicked);

    // Translation method radio button connections
    connect(ruleBasedRadio, &QRadioButton::toggled, this, &SemanticAnalyzerWidget::onTranslationMethodChanged);
    connect(mlBasedRadio, &QRadioButton::toggled, this, &SemanticAnalyzerWidget::onTranslationMethodChanged);

    // ML bridge connections
    connect(mlBridge, &MLTranslationBridge::translationCompleted, this, &SemanticAnalyzerWidget::displayTranslatedCode);
    connect(mlBridge, &MLTranslationBridge::translationError, this, [this](const QString& error) {
        statusLabel->setText(QString("❌ ML Translation Error: %1").arg(error));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f8d7da; color: #721c24; border-radius: 3px; }");
    });
}


void SemanticAnalyzerWidget::onAnalyzeClicked() {
    QString sourceCode = sourceCodeEdit->toPlainText().trimmed();

    if (sourceCode.isEmpty()) {
        QMessageBox::warning(this, "Empty Input", "Please enter some source code to analyze.");
        return;
    }

    // Tokenize
    lexer->setSkipWhitespace(true);
    lexer->setSkipComments(true);

    if (!lexer->tokenize(sourceCode)) {
        QMessageBox::critical(this, "Tokenization Failed",
                              "Failed to tokenize source code. Check for lexical errors.");
        return;
    }

    // Semantic analysis
    semanticAnalyzer->setTokens(lexer->getTokens());
    bool success = semanticAnalyzer->analyzeProgram();

    // Display results
    displaySymbolTable();
    displayErrorsWarnings();

    if (success && !semanticAnalyzer->hasErrors()) {
        translateButton->setEnabled(true);
        statusLabel->setText("✅ Semantic analysis passed - Ready to translate");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d4edda; color: #155724; border-radius: 3px; }");
    } else {
        translateButton->setEnabled(false);
        statusLabel->setText(QString("❌ Semantic analysis found %1 error(s)")
                                 .arg(semanticAnalyzer->getErrors().size()));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f8d7da; color: #721c24; border-radius: 3px; }");
    }
}

void SemanticAnalyzerWidget::onTranslateClicked() {
    TargetLanguage targetLang = targetLanguageCombo->currentData().value<TargetLanguage>();

    if (mlBasedRadio->isChecked()) {
        // ML-based translation
        statusLabel->setText("🔄 Translating with ML model...");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #fff3cd; color: #856404; border-radius: 3px; }");

        QString sourceCode = sourceCodeEdit->toPlainText();
        QString targetLanguageStr = targetLanguageCombo->currentText().toLower();
        QVector<Token> tokens = lexer->getTokens();

        mlBridge->translateCode(sourceCode, targetLanguageStr, tokens);
    } else {
        // Rule-based translation (existing logic)
        codeGenerator->setTokens(lexer->getTokens());
        codeGenerator->setSymbolTable(semanticAnalyzer->getSymbolTable());
        codeGenerator->setTargetLanguage(targetLang);
        codeGenerator->setSourceCode(sourceCodeEdit->toPlainText()); // Pass original source

        QString translatedCode = codeGenerator->generate();
        displayTranslatedCode(translatedCode);

        statusLabel->setText(QString("✅ Code translated to %1 (Rule-Based)").arg(targetLanguageCombo->currentText()));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d4edda; color: #155724; border-radius: 3px; }");
    }
}

void SemanticAnalyzerWidget::onClearClicked() {
    sourceCodeEdit->clear();
    symbolTableWidget->setRowCount(0);
    errorsWarningsText->clear();
    translatedCodeEdit->clear();
    translateButton->setEnabled(false);
    statusLabel->setText("Ready");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
}

void SemanticAnalyzerWidget::displaySymbolTable() {
    symbolTableWidget->setRowCount(0);

    if (!semanticAnalyzer) return;

    QVector<Symbol> symbols = semanticAnalyzer->getDiscoveredSymbols();

    for (const auto& symbol : symbols) {
        int row = symbolTableWidget->rowCount();
        symbolTableWidget->insertRow(row);

        QTableWidgetItem* nameItem = new QTableWidgetItem(symbol.name);
        QTableWidgetItem* typeItem = new QTableWidgetItem(symbol.getTypeString());
        QTableWidgetItem* valueItem = new QTableWidgetItem(
            symbol.isInitialized ? symbol.value : "(uninitialized)"
            );
        QTableWidgetItem* scopeItem = new QTableWidgetItem(QString::number(symbol.scope));
        QTableWidgetItem* statusItem = new QTableWidgetItem(
            symbol.isInitialized ? "Initialized" : "Declared"
            );

        // Set colors
        nameItem->setForeground(Qt::white);
        typeItem->setForeground(Qt::white);
        valueItem->setForeground(Qt::white);
        scopeItem->setForeground(Qt::white);
        statusItem->setForeground(Qt::white);

        QColor bgColor = symbol.isInitialized ? QColor(50, 150, 50) : QColor(150, 120, 50);
        nameItem->setBackground(bgColor);
        typeItem->setBackground(bgColor);
        valueItem->setBackground(bgColor);
        scopeItem->setBackground(bgColor);
        statusItem->setBackground(bgColor);

        symbolTableWidget->setItem(row, 0, nameItem);
        symbolTableWidget->setItem(row, 1, typeItem);
        symbolTableWidget->setItem(row, 2, valueItem);
        symbolTableWidget->setItem(row, 3, scopeItem);
        symbolTableWidget->setItem(row, 4, statusItem);
    }
}

void SemanticAnalyzerWidget::displayErrorsWarnings() {
    errorsWarningsText->clear();

    QString output;

    QVector<SemanticError> errors = semanticAnalyzer->getErrors();
    QVector<SemanticError> warnings = semanticAnalyzer->getWarnings();

    if (errors.isEmpty() && warnings.isEmpty()) {
        output = "<span style='color: #4ec9b0;'><b>✅ No errors or warnings!</b></span>";
    } else {
        if (!errors.isEmpty()) {
            output += "<b style='color: #f48771;'>Errors:</b><br>";
            for (const auto& error : errors) {
                output += QString("<span style='color: #f48771;'>• %1</span><br>")
                              .arg(error.toString()); // FIXED: Now uses the toString method
            }
            output += "<br>";
        }

        if (!warnings.isEmpty()) {
            output += "<b style='color: #ffc107;'>Warnings:</b><br>";
            for (const auto& warning : warnings) {
                output += QString("<span style='color: #ffc107;'>⚠ %1</span><br>")
                              .arg(warning.toString()); // FIXED: Now uses the toString method
            }
        }
    }

    errorsWarningsText->setHtml(output);
}

void SemanticAnalyzerWidget::displayTranslatedCode(const QString& code) {
    translatedCodeEdit->setPlainText(code);
}

void SemanticAnalyzerWidget::onTranslationMethodChanged() {
    if (mlBasedRadio->isChecked()) {
        statusLabel->setText("ML Translation selected - Ensure ML server is running");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #cce5ff; color: #004085; border-radius: 3px; }");
    } else {
        statusLabel->setText("Rule-Based Generator selected");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    }
}
