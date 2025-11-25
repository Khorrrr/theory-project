#include "ParserWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>
#include <QInputDialog>

ParserWidget::ParserWidget(QWidget *parent)
    : QWidget(parent), currentGrammar(nullptr), automatonManager(nullptr) {

    currentGrammar = new Grammar();
    parser = new Parser(currentGrammar);
    lexer = new Lexer();

    setupUI();
    createConnections();

    // Load default grammar
    loadPredefinedGrammar("Expression");
}

ParserWidget::~ParserWidget() {
    delete currentGrammar;
    delete parser;
    delete lexer;
}

void ParserWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void ParserWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    QLabel* titleLabel = new QLabel("Grammar Parser & Parse Tree Generator");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Main splitter (horizontal)
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);

    // ===== LEFT PANEL: Grammar Definition =====
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    // Grammar selection
    QGroupBox* grammarSelectGroup = new QGroupBox("Grammar Selection");
    QVBoxLayout* selectLayout = new QVBoxLayout();

    QHBoxLayout* selectorLayout = new QHBoxLayout();
    grammarSelector = new QComboBox();
    grammarSelector->addItem("Expression Grammar (LL)");
    grammarSelector->addItem("Arithmetic Grammar");
    grammarSelector->addItem("Statement Grammar");
    grammarSelector->addItem("Custom Grammar");
    selectorLayout->addWidget(new QLabel("Predefined:"));
    selectorLayout->addWidget(grammarSelector);

    loadGrammarBtn = new QPushButton("Load");
    loadGrammarBtn->setMaximumWidth(80);
    selectorLayout->addWidget(loadGrammarBtn);
    selectLayout->addLayout(selectorLayout);

    grammarSelectGroup->setLayout(selectLayout);
    leftLayout->addWidget(grammarSelectGroup);

    // Productions
    QGroupBox* productionsGroup = new QGroupBox("Grammar Productions");
    QVBoxLayout* prodLayout = new QVBoxLayout();

    productionsList = new QListWidget();
    productionsList->setMaximumHeight(200);
    productionsList->setStyleSheet(
        "QListWidget { background-color: #1e1e1e; color: white; }"
        "QListWidget::item { padding: 5px; }"
        "QListWidget::item:selected { background-color: #0078d7; }"
        );
    prodLayout->addWidget(productionsList);

    QHBoxLayout* prodInputLayout = new QHBoxLayout();
    productionInput = new QLineEdit();
    productionInput->setPlaceholderText("E → T E'  or  E -> T + E");
    prodInputLayout->addWidget(productionInput);

    addProductionBtn = new QPushButton("Add");
    addProductionBtn->setMaximumWidth(60);
    prodInputLayout->addWidget(addProductionBtn);

    QPushButton* deleteProductionBtn = new QPushButton("Delete");
    deleteProductionBtn->setMaximumWidth(60);
    connect(deleteProductionBtn, &QPushButton::clicked, this, &ParserWidget::onDeleteProduction);
    prodInputLayout->addWidget(deleteProductionBtn);

    prodLayout->addLayout(prodInputLayout);

    clearGrammarBtn = new QPushButton("Clear All Productions");
    prodLayout->addWidget(clearGrammarBtn);

    productionsGroup->setLayout(prodLayout);
    leftLayout->addWidget(productionsGroup);

    // Grammar info
    QGroupBox* infoGroup = new QGroupBox("Grammar Information");
    QVBoxLayout* infoLayout = new QVBoxLayout();

    grammarInfoText = new QTextEdit();
    grammarInfoText->setReadOnly(true);
    grammarInfoText->setMaximumHeight(150);
    grammarInfoText->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: white; }");
    infoLayout->addWidget(grammarInfoText);

    infoGroup->setLayout(infoLayout);
    leftLayout->addWidget(infoGroup);

    leftPanel->setMaximumWidth(400);
    mainSplitter->addWidget(leftPanel);

    // ===== RIGHT PANEL: Parsing & Tree =====
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    // Input section
    QGroupBox* inputGroup = new QGroupBox("Input to Parse");
    QVBoxLayout* inputLayout = new QVBoxLayout();

    inputTextEdit = new QTextEdit();
    inputTextEdit->setPlaceholderText("Enter expression to parse, e.g., id + id * id");
    inputTextEdit->setMaximumHeight(80);
    inputTextEdit->setText("id + id * id");
    inputLayout->addWidget(inputTextEdit);

    QHBoxLayout* parseButtonsLayout = new QHBoxLayout();
    parseButton = new QPushButton("🔍 Parse");
    parseButton->setStyleSheet("QPushButton { padding: 8px 20px; font-size: 12pt; font-weight: bold; }");
    clearButton = new QPushButton("🗑️ Clear");

    parseButtonsLayout->addStretch();
    parseButtonsLayout->addWidget(parseButton);
    parseButtonsLayout->addWidget(clearButton);
    parseButtonsLayout->addStretch();

    inputLayout->addLayout(parseButtonsLayout);
    inputGroup->setLayout(inputLayout);
    rightLayout->addWidget(inputGroup);

    // Parse output
    QGroupBox* outputGroup = new QGroupBox("Parse Output");
    QVBoxLayout* outputLayout = new QVBoxLayout();

    parseOutputText = new QTextEdit();
    parseOutputText->setReadOnly(true);
    parseOutputText->setMaximumHeight(100);
    parseOutputText->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: white; }");
    outputLayout->addWidget(parseOutputText);

    outputGroup->setLayout(outputLayout);
    rightLayout->addWidget(outputGroup);

    // Parse tree
    QGroupBox* treeGroup = new QGroupBox("Parse Tree Visualization");
    QVBoxLayout* treeLayout = new QVBoxLayout();

    parseTreeWidget = new ParseTreeWidget();
    treeLayout->addWidget(parseTreeWidget);

    treeGroup->setLayout(treeLayout);
    rightLayout->addWidget(treeGroup);

    mainSplitter->addWidget(rightPanel);
    mainLayout->addWidget(mainSplitter);

    // Status
    statusLabel = new QLabel("Ready - Select or define a grammar");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
    mainLayout->addWidget(statusLabel);

    setLayout(mainLayout);
}

void ParserWidget::createConnections() {
    connect(loadGrammarBtn, &QPushButton::clicked, this, &ParserWidget::onLoadGrammar);
    connect(addProductionBtn, &QPushButton::clicked, this, &ParserWidget::onAddProduction);
    connect(clearGrammarBtn, &QPushButton::clicked, this, &ParserWidget::onClearGrammar);
    connect(parseButton, &QPushButton::clicked, this, &ParserWidget::onParseClicked);
    connect(clearButton, &QPushButton::clicked, this, &ParserWidget::onClearClicked);
    connect(productionInput, &QLineEdit::returnPressed, this, &ParserWidget::onAddProduction);
}

void ParserWidget::onLoadGrammar() {
    QString selected = grammarSelector->currentText();

    if (selected.contains("Expression")) {
        loadPredefinedGrammar("Expression");
    } else if (selected.contains("Arithmetic")) {
        loadPredefinedGrammar("Arithmetic");
    } else if (selected.contains("Statement")) {
        loadPredefinedGrammar("Statement");
    } else {
        currentGrammar->clear();
        currentGrammar->setName("Custom Grammar");
        currentGrammar->setStartSymbol("S");
    }

    updateGrammarDisplay();
    updateProductionsList();
    statusLabel->setText(QString("Loaded: %1").arg(currentGrammar->getName()));
}

void ParserWidget::loadPredefinedGrammar(const QString& grammarName) {
    delete currentGrammar;

    if (grammarName == "Expression") {
        currentGrammar = new Grammar(Grammar::createExpressionGrammar());
    } else if (grammarName == "Arithmetic") {
        currentGrammar = new Grammar(Grammar::createArithmeticGrammar());
    } else if (grammarName == "Statement") {
        currentGrammar = new Grammar(Grammar::createSimpleStatementGrammar());
    } else {
        currentGrammar = new Grammar("Custom", "S");
    }

    parser->setGrammar(currentGrammar);
}

void ParserWidget::onAddProduction() {
    QString prodStr = productionInput->text().trimmed();

    if (prodStr.isEmpty()) {
        QMessageBox::warning(this, "Empty Production", "Please enter a production rule.");
        return;
    }

    Production prod = Production::fromString(prodStr);

    if (prod.getNonTerminal().isEmpty()) {
        QMessageBox::warning(this, "Invalid Production",
                             "Invalid production format.\n\nUse: A → B C  or  A -> B C");
        return;
    }

    currentGrammar->addProduction(prod);
    updateGrammarDisplay();
    updateProductionsList();

    productionInput->clear();
    statusLabel->setText(QString("Added: %1").arg(prod.toString()));
}

void ParserWidget::onDeleteProduction() {
    int currentRow = productionsList->currentRow();
    if (currentRow >= 0) {
        currentGrammar->removeProduction(currentRow);
        updateGrammarDisplay();
        updateProductionsList();
        statusLabel->setText("Production deleted");
    }
}

void ParserWidget::onClearGrammar() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Clear Grammar", "Clear all productions?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        currentGrammar->clear();
        updateGrammarDisplay();
        updateProductionsList();
        statusLabel->setText("Grammar cleared");
    }
}

void ParserWidget::onParseClicked() {
    if (!currentGrammar || currentGrammar->getProductions().isEmpty()) {
        QMessageBox::warning(this, "No Grammar", "Please load or define a grammar first.");
        return;
    }

    QString input = inputTextEdit->toPlainText().trimmed();

    if (input.isEmpty()) {
        QMessageBox::warning(this, "Empty Input", "Please enter some input to parse.");
        return;
    }

    // Tokenize input first
    lexer->setSkipWhitespace(true);
    lexer->setSkipComments(true);

    if (!lexer->tokenize(input)) {
        displayParseErrors(QVector<ParseError>());
        parseOutputText->append("\n❌ Tokenization failed!");
        return;
    }

    QVector<Token> tokens = lexer->getTokens();
    parser->setTokens(tokens);

    // Parse
    ParseTree tree = parser->parse();

    // Display results
    displayParseResult(tree);

    if (parser->hasErrors()) {
        displayParseErrors(parser->getErrors());
    }
}

void ParserWidget::onClearClicked() {
    inputTextEdit->clear();
    parseOutputText->clear();
    parseTreeWidget->clear();
    statusLabel->setText("Ready");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e9ecef; border-radius: 3px; }");
}

void ParserWidget::onProductionSelected(int index) {
    Q_UNUSED(index);
}

void ParserWidget::updateGrammarDisplay() {
    if (!currentGrammar) return;

    QString info;
    info += QString("<b>Grammar:</b> %1<br>").arg(currentGrammar->getName());
    info += QString("<b>Start Symbol:</b> %1<br><br>").arg(currentGrammar->getStartSymbol());

    QSet<QString> terminals = currentGrammar->getTerminals();
    QSet<QString> nonTerminals = currentGrammar->getNonTerminals();

    info += QString("<b>Non-Terminals:</b> { %1 }<br>")
                .arg(QStringList(nonTerminals.values()).join(", "));

    info += QString("<b>Terminals:</b> { %1 }<br>")
                .arg(QStringList(terminals.values()).join(", "));

    info += QString("<br><b>Total Productions:</b> %1")
                .arg(currentGrammar->getProductions().size());

    grammarInfoText->setHtml(info);
}

void ParserWidget::updateProductionsList() {
    productionsList->clear();

    if (!currentGrammar) return;

    for (const auto& prod : currentGrammar->getProductions()) {
        productionsList->addItem(prod.toString());
    }
}

void ParserWidget::displayParseResult(const ParseTree& tree) {
    parseOutputText->clear();

    QString output;
    output += QString("<b style='color: #0078d7;'>Parsing Result</b><br>");
    output += QString("<b>Input:</b> %1<br>").arg(inputTextEdit->toPlainText());
    output += QString("<b>Grammar:</b> %1<br><br>").arg(currentGrammar->getName());

    if (tree.isEmpty()) {
        output += "<span style='color: red;'><b>❌ Parse Failed!</b></span><br>";
        statusLabel->setText("❌ Parse Failed");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #f8d7da; color: #721c24; border-radius: 3px; }");
    } else {
        output += "<span style='color: green;'><b>✅ Parse Successful!</b></span><br>";
        output += "<br><b>Parse Tree:</b><br>";
        output += "<pre style='color: white;'>" + tree.toString() + "</pre>";

        parseTreeWidget->setParseTree(tree);

        statusLabel->setText("✅ Parse Successful - Tree displayed");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #d4edda; color: #155724; border-radius: 3px; }");
    }

    parseOutputText->setHtml(output);
}

void ParserWidget::displayParseErrors(const QVector<ParseError>& errors) {
    if (errors.isEmpty()) return;

    QString errorOutput = "<br><b style='color: red;'>Parse Errors:</b><br>";

    for (const auto& error : errors) {
        errorOutput += QString("<span style='color: #ff6b6b;'>• %1</span><br>")
                           .arg(error.toString());
    }

    parseOutputText->append(errorOutput);
}
