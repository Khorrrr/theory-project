#include "MainWindow.h" // Includes the main window class definition.
#include "./src/utils/Automaton/NFAtoDFA.h" // Provides functionality to convert NFA to DFA.
#include "./src/utils/Automaton/DFAMinimizer.h" // Provides functionality to minimize DFA.
#include <QInputDialog> // For getting single-line text input from the user.
#include <QFileDialog>  // For file open/save dialogs (currently placeholders).
#include <QDialog>      // Base class for dialog windows.
#include <QCheckBox>    // For checkbox widgets.
#include <QtMath>       // For mathematical functions like qCeil and qSqrt used in layout calculations.
#include <QScrollArea>  // Provides a scrollable view for other widgets.
#include <QButtonGroup> // Manages a group of buttons (e.g., radio buttons) to ensure exclusivity.
#include <QHeaderView>  // For customizing table headers.
#include <QDebug>       // For debugging output (qDebug(), qWarning(), qCritical()).

// Constructor for the MainWindow class.
// Initializes the main application window and its components.
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    // Initialize pointers to nullptr or default values to ensure a clean state
    // and proper memory management.
    currentAutomaton(nullptr), automatonCounter(0),
    currentSelectedStateId(""), automatonManager(nullptr), canvas(nullptr),
    centralTabs(nullptr), automatonTab(nullptr), lexerWidget(nullptr),
    toolsDock(nullptr), automatonListDock(nullptr), propertiesDock(nullptr),
    testingDock(nullptr), automatonList(nullptr), testResultsText(nullptr),
    typeLabel(nullptr), stateCountLabel(nullptr), transitionCountLabel(nullptr),
    alphabetLabel(nullptr), selectedStateLabel(nullptr), deleteStateBtn(nullptr),
    transitionTable(nullptr), convertNFAtoDFABtn(nullptr), minimizeDFABtn(nullptr),
    testInputField(nullptr), testInputBtn(nullptr), clearTestBtn(nullptr),
    selectModeBtn(nullptr), addStateModeBtn(nullptr), addTransitionModeBtn(nullptr),
    deleteModeBtn(nullptr), clearCanvasBtn(nullptr), newAutomatonBtn(nullptr),
    deleteAutomatonBtn(nullptr), renameAutomatonBtn(nullptr),
    newAction(nullptr), openAction(nullptr), saveAction(nullptr), exitAction(nullptr),
    convertAction(nullptr), minimizeAction(nullptr), aboutAction(nullptr),
    selectAction(nullptr), addStateAction(nullptr), addTransitionAction(nullptr),
    deleteAction(nullptr) {

    // Set the window title and initial size.
    setWindowTitle("Compiler Project");
    resize(1200, 700);
    setMinimumSize(1000, 650); // Ensures the window cannot be made too small.

    // Initialize the AutomatonManager first, as other widgets depend on it.
    automatonManager = new AutomatonManager();
    if (!automatonManager) {
        qCritical() << "Failed to create AutomatonManager!"; // Log critical error if creation fails.
        return; // Exit if a critical component cannot be initialized.
    }

    // Setup the central tab widget which houses the different analysis tools.
    // This must be done before creating menus and dock widgets that might interact with these tabs.
    setupCentralTabs();

    // Create the application's menu bar (File, Tools, Help).
    createMenus();
    // Create and arrange the dockable widgets (Tools, Automaton List, Properties, Test Input).
    createDockWidgets();

    // Connect signals from the automaton canvas to slots in MainWindow.
    // This allows the main window to react to changes on the canvas (e.g., state selected, automaton modified).
    if (canvas) {
        connect(canvas, &AutomatonCanvas::automatonModified,
                this, &MainWindow::onAutomatonModified);
        connect(canvas, &AutomatonCanvas::stateSelected,
                this, &MainWindow::onStateSelected);
    } else {
        qWarning() << "Canvas is null - connections failed"; // Log warning if canvas is not available.
    }

    // Display an initial welcome message and instructions in the test results area.
    if (testResultsText) {
        testResultsText->setHtml(
            "<div style='color: black; padding: 10px;'>"
            "<h3 style='color: #0066cc;'>Welcome to Compiler Project!</h3>"
            "<p><b>Automaton Designer Tab:</b></p>"
            "<ol>"
            "<li>Click <b>'New'</b> to create DFA/NFA</li>"
            "<li>Add states and transitions</li>"
            "<li>Test with input strings</li>"
            "</ol>"
            "<p><b>Lexical Analyzer Tab:</b></p>"
            "<p>Switch tabs to tokenize source code!</p>"
            "</div>"
            );
    }

    // Set the initial message in the status bar.
    statusBar()->showMessage("Ready - Click 'New' or switch to Lexical Analyzer tab");
}

// Destructor for the MainWindow class.
// This is responsible for cleaning up dynamically allocated resources
// to prevent memory leaks.
MainWindow::~MainWindow() {
    // Disconnect signals from canvas and central tabs to prevent
    // dangling pointers or crashes during shutdown, especially if
    // connected objects are deleted first.
    if (canvas) {
        canvas->disconnect();
    }

    if (centralTabs) {
        centralTabs->disconnect();
    }

    // Delete all Automaton objects stored in the map.
    // Iterates through each value (Automaton*) in the 'automatons' QMap.
    for (auto automaton : automatons) {
        if (automaton) {
            delete automaton; // Deallocates the memory for each Automaton object.
        }
    }
    automatons.clear(); // Clears the map itself, removing all key-value pairs.

    // Reset pointers to nullptr to avoid dangling pointers.
    currentAutomaton = nullptr;

    // Delete the AutomatonManager last, as it might be managing resources
    // that other widgets (like LexerWidget, ParserWidget, SemanticAnalyzerWidget)
    // might still briefly refer to during their own destruction, although their
    // direct dependencies are ideally nulled out or disconnected earlier.
    if (automatonManager) {
        delete automatonManager;
        automatonManager = nullptr;
    }

    // Qt's parent-child mechanism typically handles the deletion of UI widgets
    // (like centralTabs, toolsDock, etc.) that have `this` (MainWindow) as their parent.
    // Therefore, explicit `delete` calls are usually not needed for these members
    // if they were created with `this` as parent.
}

void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu("&File");

    newAction = new QAction("&New Automaton", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNew);
    fileMenu->addAction(newAction);

    openAction = new QAction("&Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpen);
    fileMenu->addAction(openAction);

    saveAction = new QAction("&Save...", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);
    fileMenu->addAction(saveAction);

    fileMenu->addSeparator();

    exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAction);

    QMenu* toolsMenu = menuBar()->addMenu("&Tools");

    convertAction = new QAction("Convert NFA to DFA", this);
    convertAction->setShortcut(Qt::CTRL + Qt::Key_T);
    connect(convertAction, &QAction::triggered, this, &MainWindow::onConvertNFAtoDFA);
    toolsMenu->addAction(convertAction);

    minimizeAction = new QAction("Minimize DFA", this);
    minimizeAction->setShortcut(Qt::CTRL + Qt::Key_M);
    connect(minimizeAction, &QAction::triggered, this, &MainWindow::onMinimizeDFA);
    toolsMenu->addAction(minimizeAction);

    QMenu* helpMenu = menuBar()->addMenu("&Help");

    aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::createDockWidgets() {
    createToolsPanel();
    createAutomatonListPanel();
    createPropertiesPanel();
    createTestingPanel();
}

void MainWindow::createToolsPanel() {
    toolsDock = new QDockWidget("Tools", this);
    toolsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* toolsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QGroupBox* modeGroup = new QGroupBox("Drawing Mode");
    QVBoxLayout* modeLayout = new QVBoxLayout();
    modeLayout->setSpacing(3);

    selectModeBtn = new QRadioButton("Select");
    selectModeBtn->setChecked(true);
    connect(selectModeBtn, &QRadioButton::clicked, this, &MainWindow::onSelectMode);
    modeLayout->addWidget(selectModeBtn);

    addStateModeBtn = new QRadioButton("Add State");
    connect(addStateModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddStateMode);
    modeLayout->addWidget(addStateModeBtn);

    addTransitionModeBtn = new QRadioButton("Add Transition");
    connect(addTransitionModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddTransitionMode);
    modeLayout->addWidget(addTransitionModeBtn);

    deleteModeBtn = new QRadioButton("Delete");
    connect(deleteModeBtn, &QRadioButton::clicked, this, &MainWindow::onDeleteMode);
    modeLayout->addWidget(deleteModeBtn);

    modeGroup->setLayout(modeLayout);
    layout->addWidget(modeGroup);

    QGroupBox* actionsGroup = new QGroupBox("Actions");
    QVBoxLayout* actionsLayout = new QVBoxLayout();
    actionsLayout->setSpacing(3);

    clearCanvasBtn = new QPushButton("Clear Canvas");
    connect(clearCanvasBtn, &QPushButton::clicked, this, &MainWindow::onClearCanvas);
    actionsLayout->addWidget(clearCanvasBtn);

    actionsGroup->setLayout(actionsLayout);
    layout->addWidget(actionsGroup);

    layout->addStretch();

    toolsWidget->setLayout(layout);
    toolsWidget->setMaximumWidth(150);
    toolsDock->setWidget(toolsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, toolsDock);
}

void MainWindow::createAutomatonListPanel() {
    automatonListDock = new QDockWidget("Automatons", this);
    automatonListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* listWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    automatonList = new QListWidget();
    automatonList->setMaximumHeight(120);
    connect(automatonList, &QListWidget::itemClicked,
            this, &MainWindow::onAutomatonSelected);
    layout->addWidget(automatonList);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(3);

    newAutomatonBtn = new QPushButton("New");
    newAutomatonBtn->setMaximumWidth(50);
    connect(newAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onNewAutomaton);
    btnLayout->addWidget(newAutomatonBtn);

    deleteAutomatonBtn = new QPushButton("Del");
    deleteAutomatonBtn->setMaximumWidth(50);
    connect(deleteAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onDeleteAutomaton);
    btnLayout->addWidget(deleteAutomatonBtn);

    renameAutomatonBtn = new QPushButton("Rename");
    connect(renameAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onRenameAutomaton);
    btnLayout->addWidget(renameAutomatonBtn);

    layout->addLayout(btnLayout);

    listWidget->setLayout(layout);
    listWidget->setMaximumWidth(150);
    automatonListDock->setWidget(listWidget);
    addDockWidget(Qt::LeftDockWidgetArea, automatonListDock);
}

void MainWindow::createPropertiesPanel() {
    propertiesDock = new QDockWidget("Properties", this);
    propertiesDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    QWidget* propsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    // General automaton info
    QGroupBox* infoGroup = new QGroupBox("Automaton Info");
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    typeLabel = new QLabel("Type: N/A");
    typeLabel->setStyleSheet("font-weight: bold;");
    infoLayout->addWidget(typeLabel);

    stateCountLabel = new QLabel("States: 0");
    infoLayout->addWidget(stateCountLabel);

    transitionCountLabel = new QLabel("Transitions: 0");
    infoLayout->addWidget(transitionCountLabel);

    alphabetLabel = new QLabel("Alphabet: {}");
    alphabetLabel->setWordWrap(true);
    infoLayout->addWidget(alphabetLabel);

    infoGroup->setLayout(infoLayout);
    layout->addWidget(infoGroup);

    // Selected state info
    selectedStateLabel = new QLabel("No state selected");
    selectedStateLabel->setStyleSheet(
        "background-color: #f5f5f5; "
        "color: #999; "
        "padding: 8px; "
        "border: 1px solid #ddd; "
        "border-radius: 3px;"
        );
    selectedStateLabel->setAlignment(Qt::AlignCenter);
    selectedStateLabel->setWordWrap(true);
    layout->addWidget(selectedStateLabel);

    // Delete button for selected state
    deleteStateBtn = new QPushButton("🗑 Delete Options");
    deleteStateBtn->setVisible(false);
    deleteStateBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #dc3545; "
        "   color: white; "
        "   border: none; "
        "   padding: 10px; "
        "   font-weight: bold; "
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #c82333; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #bd2130; "
        "}"
        );
    connect(deleteStateBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStateOrTransition);
    layout->addWidget(deleteStateBtn);

    // All transitions table
    QGroupBox* allTransGroup = new QGroupBox("All Transitions");
    QVBoxLayout* allTransLayout = new QVBoxLayout();
    allTransLayout->setSpacing(3);

    transitionTable = new QTableWidget();
    transitionTable->setColumnCount(3);
    transitionTable->setHorizontalHeaderLabels({"From", "Symbol", "To"});
    transitionTable->horizontalHeader()->setStretchLastSection(true);
    transitionTable->setMaximumHeight(120);
    transitionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    allTransLayout->addWidget(transitionTable);

    allTransGroup->setLayout(allTransLayout);
    layout->addWidget(allTransGroup);

    // Conversion and Minimization buttons
    convertNFAtoDFABtn = new QPushButton("Convert NFA → DFA");
    convertNFAtoDFABtn->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(convertNFAtoDFABtn, &QPushButton::clicked, this, &MainWindow::onConvertNFAtoDFA);
    layout->addWidget(convertNFAtoDFABtn);

    minimizeDFABtn = new QPushButton("⚡ Minimize DFA");
    minimizeDFABtn->setStyleSheet(
        "QPushButton { background-color: #ffc107; color: black; border: none; padding: 8px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #ffb300; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    connect(minimizeDFABtn, &QPushButton::clicked, this, &MainWindow::onMinimizeDFA);
    layout->addWidget(minimizeDFABtn);

    layout->addStretch();

    propsWidget->setLayout(layout);
    propsWidget->setMaximumWidth(250);
    propertiesDock->setWidget(propsWidget);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);
}

void MainWindow::createTestingPanel() {
    testingDock = new QDockWidget("Test Input", this);
    testingDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

    QWidget* testWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* inputLayout = new QHBoxLayout();

    QLabel* label = new QLabel("Input:");
    inputLayout->addWidget(label);

    testInputField = new QLineEdit();
    testInputField->setPlaceholderText("e.g., 010101");
    connect(testInputField, &QLineEdit::returnPressed, this, &MainWindow::onTestInput);
    inputLayout->addWidget(testInputField);

    testInputBtn = new QPushButton("Test");
    testInputBtn->setMaximumWidth(60);
    connect(testInputBtn, &QPushButton::clicked, this, &MainWindow::onTestInput);
    inputLayout->addWidget(testInputBtn);

    clearTestBtn = new QPushButton("Clear");
    clearTestBtn->setMaximumWidth(60);
    connect(clearTestBtn, &QPushButton::clicked, this, &MainWindow::onClearTest);
    inputLayout->addWidget(clearTestBtn);

    layout->addLayout(inputLayout);

    testResultsText = new QTextEdit();
    testResultsText->setReadOnly(true);
    testResultsText->setMaximumHeight(80);
    layout->addWidget(testResultsText);

    testWidget->setLayout(layout);
    testingDock->setWidget(testWidget);
    testingDock->setMaximumHeight(150);
    addDockWidget(Qt::BottomDockWidgetArea, testingDock);
}

// Mode selection slots
void MainWindow::onSelectMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::Select);
    if (selectModeBtn) selectModeBtn->setChecked(true);
    if (selectAction) selectAction->setChecked(true);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Select/Move mode - Click states to see delete options");
}

void MainWindow::onAddStateMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::AddState);
    if (addStateModeBtn) addStateModeBtn->setChecked(true);
    if (addStateAction) addStateAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Add State mode - Click to add states");
}

void MainWindow::onAddTransitionMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::AddTransition);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(true);
    if (addTransitionAction) addTransitionAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Add Transition mode - Click two states to connect");
}

void MainWindow::onDeleteMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::Delete);
    if (deleteModeBtn) deleteModeBtn->setChecked(true);
    if (deleteAction) deleteAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    statusBar()->showMessage("Delete mode - Click to delete states");
}

void MainWindow::onNewAutomaton() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create New Automaton");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: white; background-color: transparent; }"
        "QRadioButton { "
        "   color: white; "
        "   background-color: transparent; "
        "   spacing: 8px; "
        "   padding: 5px;"
        "}"
        "QRadioButton::indicator { "
        "   width: 18px; "
        "   height: 18px; "
        "   border-radius: 9px;"
        "   border: 2px solid #888;"
        "   background-color: #1e1e1e;"
        "}"
        "QRadioButton::indicator:checked { "
        "   background-color: #0078d7; "
        "   border: 2px solid #0078d7;"
        "}"
        "QRadioButton::indicator:hover { "
        "   border: 2px solid #0078d7;"
        "}"
        "QPushButton { "
        "   color: white; "
        "   background-color: #3e3e3e; "
        "   border: 1px solid #555; "
        "   padding: 8px 20px; "
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #4e4e4e; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #2e2e2e; "
        "}"
        "QGroupBox { "
        "   color: white; "
        "   font-weight: bold; "
        "   border: 2px solid #555; "
        "   border-radius: 5px; "
        "   margin-top: 15px; "
        "   padding-top: 10px; "
        "   background-color: #1e1e1e;"
        "}"
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 5px; "
        "   background-color: #2b2b2b;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel("Select Automaton Type:");
    titleLabel->setStyleSheet("color: white; font-size: 13pt; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);

    QGroupBox* typeGroup = new QGroupBox("Automaton Type");
    typeGroup->setStyleSheet(
        "QGroupBox { "
        "   color: white; "
        "   font-weight: bold; "
        "   border: 2px solid #0078d7; "
        "   border-radius: 5px; "
        "   margin-top: 15px; "
        "   padding: 15px; "
        "   background-color: #1e1e1e;"
        "}"
        "QGroupBox::title { "
        "   subcontrol-origin: margin; "
        "   left: 10px; "
        "   padding: 0 8px; "
        "   background-color: #2b2b2b;"
        "   color: #0078d7;"
        "}"
        );
    QVBoxLayout* typeLayout = new QVBoxLayout();
    typeLayout->setSpacing(15);

    QRadioButton* dfaRadio = new QRadioButton("DFA (Deterministic Finite Automaton)");
    dfaRadio->setStyleSheet(
        "QRadioButton { "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 11pt;"
        "   spacing: 8px; "
        "   padding: 5px;"
        "}"
        "QRadioButton::indicator { "
        "   width: 20px; "
        "   height: 20px; "
        "   border-radius: 10px;"
        "   border: 2px solid #888;"
        "   background-color: #1e1e1e;"
        "}"
        "QRadioButton::indicator:checked { "
        "   background-color: #0078d7; "
        "   border: 3px solid #0078d7;"
        "}"
        "QRadioButton::indicator:hover { "
        "   border: 2px solid #0078d7;"
        "}"
        );
    dfaRadio->setChecked(true);
    typeLayout->addWidget(dfaRadio);

    QLabel* dfaDesc = new QLabel(
        "• Each state has exactly ONE transition per symbol\n"
        "• No epsilon (E) transitions allowed\n"
        "• Deterministic - predictable behavior"
        );
    dfaDesc->setStyleSheet(
        "color: #ccc; "
        "font-size: 10pt; "
        "margin-left: 30px; "
        "background-color: #252525; "
        "padding: 8px; "
        "border-left: 3px solid #0078d7;"
        );
    typeLayout->addWidget(dfaDesc);

    typeLayout->addSpacing(10);

    QRadioButton* nfaRadio = new QRadioButton("NFA (Non-deterministic Finite Automaton)");
    nfaRadio->setStyleSheet(
        "QRadioButton { "
        "   color: white; "
        "   font-weight: bold; "
        "   font-size: 11pt;"
        "   spacing: 8px; "
        "   padding: 5px;"
        "}"
        "QRadioButton::indicator { "
        "   width: 20px; "
        "   height: 20px; "
        "   border-radius: 10px;"
        "   border: 2px solid #888;"
        "   background-color: #1e1e1e;"
        "}"
        "QRadioButton::indicator:checked { "
        "   background-color: #28a745; "
        "   border: 3px solid #28a745;"
        "}"
        "QRadioButton::indicator:hover { "
        "   border: 2px solid #28a745;"
        "}"
        );
    typeLayout->addWidget(nfaRadio);

    QLabel* nfaDesc = new QLabel(
        "• States can have MULTIPLE transitions per symbol\n"
        "• Epsilon (E) transitions allowed\n"
        "• Non-deterministic - multiple possible paths"
        );
    nfaDesc->setStyleSheet(
        "color: #ccc; "
        "font-size: 10pt; "
        "margin-left: 30px; "
        "background-color: #252525; "
        "padding: 8px; "
        "border-left: 3px solid #28a745;"
        );
    typeLayout->addWidget(nfaDesc);

    typeGroup->setLayout(typeLayout);
    layout->addWidget(typeGroup);

    layout->addSpacing(10);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* createBtn = new QPushButton("✓ Create");
    createBtn->setStyleSheet(
        "QPushButton { "
        "   color: white; "
        "   background-color: #28a745; "
        "   border: none; "
        "   padding: 10px 25px; "
        "   font-weight: bold;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #218838; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1e7e34; "
        "}"
        );

    QPushButton* cancelBtn = new QPushButton("✗ Cancel");
    cancelBtn->setStyleSheet(
        "QPushButton { "
        "   color: white; "
        "   background-color: #dc3545; "
        "   border: none; "
        "   padding: 10px 25px; "
        "   font-weight: bold;"
        "   border-radius: 3px;"
        "}"
        "QPushButton:hover { "
        "   background-color: #c82333; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #bd2130; "
        "}"
        );

    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(createBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        AutomatonType type = dfaRadio->isChecked() ? AutomatonType::DFA : AutomatonType::NFA;

        QString id = generateAutomatonId();
        QString name = QString("%1 %2").arg(type == AutomatonType::DFA ? "DFA" : "NFA").arg(automatonCounter);

        Automaton* newAutomaton = new Automaton(id, name, type);
        if (!newAutomaton) {
            qCritical() << "Failed to create new automaton";
            return;
        }

        automatons[id] = newAutomaton;

        updateAutomatonList();

        for (int i = 0; i < automatonList->count(); ++i) {
            QListWidgetItem* item = automatonList->item(i);
            if (item && item->data(Qt::UserRole).toString() == id) {
                automatonList->setCurrentItem(item);
                setCurrentAutomaton(newAutomaton);
                break;
            }
        }

        QString typeDesc = type == AutomatonType::DFA ?
                               "DFA created. Remember: Each state must have exactly one transition per symbol." :
                               "NFA created. You can add multiple transitions per symbol and use 'E' for epsilon.";

        statusBar()->showMessage(QString("%1 - %2").arg(name).arg(typeDesc), 5000);
    }
}

void MainWindow::onDeleteAutomaton() {
    if (!automatonList) return;

    QListWidgetItem* item = automatonList->currentItem();
    if (!item) {
        showStyledMessageBox("Warning", "Please select an automaton to delete.", QMessageBox::Warning);
        return;
    }

    QString id = item->data(Qt::UserRole).toString();

    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
        );
    msgBox.setWindowTitle("Confirm Delete");
    msgBox.setText("Are you sure you want to delete this automaton?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        if (automatons.contains(id)) {
            // Safe deletion
            if (currentAutomaton && currentAutomaton->getId() == id) {
                currentAutomaton = nullptr;
                currentSelectedStateId = "";
                if (canvas) {
                    canvas->setAutomaton(nullptr);
                }
            }

            delete automatons[id];
            automatons.remove(id);

            updateAutomatonList();
            updateProperties();
            statusBar()->showMessage("Automaton deleted");
        }
    }
}

void MainWindow::onRenameAutomaton() {
    if (!automatonList) return;

    QListWidgetItem* item = automatonList->currentItem();
    if (!item) {
        showStyledMessageBox("Warning", "Please select an automaton to rename.", QMessageBox::Warning);
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    Automaton* automaton = automatons.value(id);

    if (automaton) {
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename Automaton",
                                                "Enter new name:", QLineEdit::Normal, automaton->getName(), &ok);

        if (ok && !newName.isEmpty()) {
            automaton->setName(newName);
            updateAutomatonList();
            statusBar()->showMessage(QString("Automaton renamed to: %1").arg(newName));
        }
    }
}

void MainWindow::onAutomatonSelected(QListWidgetItem* item) {
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();
    Automaton* automaton = automatons.value(id);

    if (automaton) {
        setCurrentAutomaton(automaton);
        statusBar()->showMessage(QString("Selected: %1").arg(automaton->getName()));
    }
}

void MainWindow::onClearCanvas() {
    if (!currentAutomaton) return;

    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
        );
    msgBox.setWindowTitle("Confirm Clear");
    msgBox.setText("Are you sure you want to clear the canvas?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        currentAutomaton->clear();
        currentSelectedStateId = "";
        if (canvas) {
            canvas->update();
        }
        updateProperties();
        statusBar()->showMessage("Canvas cleared");
    }
}

void MainWindow::onDeleteStateOrTransition() {
    if (!currentAutomaton || currentSelectedStateId.isEmpty()) return;

    // Make a COPY of the state ID before any deletion occurs
    QString stateIdToDelete = currentSelectedStateId;

    const State* selectedState = currentAutomaton->getState(currentSelectedStateId);
    if (!selectedState) {
        // State no longer exists, clear selection
        currentSelectedStateId = "";
        updateProperties();
        return;
    }

    // Create custom dialog with black background
    QDialog dialog(this);
    dialog.setWindowTitle("Delete Options");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog { background-color: #2b2b2b; }"
        "QLabel { color: white; background-color: transparent; }"
        "QRadioButton { color: white; background-color: transparent; padding: 5px; }"
        "QRadioButton::indicator { width: 16px; height: 16px; border-radius: 8px; "
        "border: 2px solid #888; background-color: #1e1e1e; }"
        "QRadioButton::indicator:checked { background-color: #dc3545; border: 2px solid #dc3545; }"
        "QRadioButton::indicator:hover { border: 2px solid #dc3545; }"
        "QPushButton { color: white; background-color: #3e3e3e; border: 1px solid #555; "
        "padding: 8px 20px; border-radius: 3px; }"
        "QPushButton:hover { background-color: #4e4e4e; }"
        "QPushButton:pressed { background-color: #2e2e2e; }"
        "QListWidget { background-color: #1e1e1e; color: white; border: 1px solid #555; }"
        "QListWidget::item { padding: 5px; }"
        "QListWidget::item:selected { background-color: #0078d7; }"
        "QListWidget::item:hover { background-color: #3e3e3e; }"
        "QGroupBox { color: white; border: 1px solid #555; border-radius: 3px; "
        "margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        );

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    QLabel* titleLabel = new QLabel(QString("Delete options for state: <b>%1</b>").arg(selectedState->getLabel()));
    titleLabel->setStyleSheet("color: white; font-size: 12pt; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // Radio buttons for delete options
    QRadioButton* deleteNodeRadio = new QRadioButton("Delete entire state (node)");
    deleteNodeRadio->setStyleSheet("color: #ff6b6b; font-weight: bold; font-size: 11pt;");
    deleteNodeRadio->setChecked(true);
    mainLayout->addWidget(deleteNodeRadio);

    QLabel* nodeWarning = new QLabel("⚠ This will remove the state and ALL its transitions");
    nodeWarning->setStyleSheet("color: #ffa500; margin-left: 25px; font-size: 9pt; font-style: italic;");
    mainLayout->addWidget(nodeWarning);

    mainLayout->addSpacing(10);

    QRadioButton* deleteTransitionRadio = new QRadioButton("Delete specific transition(s)");
    deleteTransitionRadio->setStyleSheet("color: #4ec9b0; font-weight: bold; font-size: 11pt;");
    mainLayout->addWidget(deleteTransitionRadio);

    // Get all transitions from this state
    QVector<Transition> fromTransitions = currentAutomaton->getTransitionsFrom(currentSelectedStateId);

    QGroupBox* transitionsGroup = new QGroupBox("Select transitions to delete:");
    transitionsGroup->setEnabled(false);
    QVBoxLayout* transLayout = new QVBoxLayout();

    QListWidget* transitionsList = new QListWidget();
    transitionsList->setSelectionMode(QAbstractItemView::MultiSelection);

    if (fromTransitions.isEmpty()) {
        QListWidgetItem* noTransItem = new QListWidgetItem("(No outgoing transitions)");
        noTransItem->setFlags(Qt::NoItemFlags);
        transitionsList->addItem(noTransItem);
        deleteTransitionRadio->setEnabled(false);
        deleteTransitionRadio->setStyleSheet("color: #666; font-size: 11pt;");
    } else {
        for (const auto& trans : fromTransitions) {
            QString transText = QString("%1 --(%2)--> %3")
            .arg(trans.getFromStateId())
                .arg(trans.getSymbolsString())
                .arg(trans.getToStateId());

            QListWidgetItem* item = new QListWidgetItem(transText);
            item->setData(Qt::UserRole, trans.getToStateId());
            item->setData(Qt::UserRole + 1, trans.getSymbolsString());
            transitionsList->addItem(item);
        }
    }

    transLayout->addWidget(transitionsList);
    transitionsGroup->setLayout(transLayout);
    mainLayout->addWidget(transitionsGroup);

    // Enable/disable transitions list based on radio selection
    connect(deleteTransitionRadio, &QRadioButton::toggled, [transitionsGroup](bool checked) {
        transitionsGroup->setEnabled(checked);
    });

    mainLayout->addSpacing(15);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* deleteBtn = new QPushButton("🗑 Delete");
    deleteBtn->setStyleSheet(
        "QPushButton { color: white; background-color: #dc3545; border: none; "
        "padding: 10px 25px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #c82333; }"
        "QPushButton:pressed { background-color: #bd2130; }"
        );

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setStyleSheet(
        "QPushButton { color: white; background-color: #6c757d; border: none; "
        "padding: 10px 25px; font-weight: bold; border-radius: 3px; }"
        "QPushButton:hover { background-color: #5a6268; }"
        "QPushButton:pressed { background-color: #545b62; }"
        );

    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    connect(deleteBtn, &QPushButton::clicked, [&, stateIdToDelete]() { // CAPTURE BY COPY
        if (deleteNodeRadio->isChecked()) {
            // Delete entire state
            QMessageBox confirmBox(&dialog);
            confirmBox.setStyleSheet(
                "QMessageBox { background-color: #2b2b2b; }"
                "QLabel { color: white; }"
                "QPushButton { color: white; background-color: #3e3e3e; border: 1px solid #555; padding: 5px 15px; }"
                );
            confirmBox.setWindowTitle("Confirm Delete State");
            confirmBox.setText(QString("Really delete state '%1' and all its transitions?").arg(selectedState->getLabel()));
            confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmBox.setIcon(QMessageBox::Warning);

            if (confirmBox.exec() == QMessageBox::Yes) {
                QString stateId = currentSelectedStateId;
                QString stateLabel = selectedState->getLabel();

                // Clear selection BEFORE deleting to prevent accessing deleted state
                currentSelectedStateId = "";

                // Safely delete the state
                if (currentAutomaton->removeState(stateIdToDelete)) {
                    statusBar()->showMessage(QString("✓ State '%1' deleted").arg(stateLabel), 3000);
                    updateProperties();
                    if (canvas) {
                        canvas->update();
                    }
                    dialog.accept();
                } else {
                    statusBar()->showMessage("Failed to delete state", 3000);
                }
            }
        } else {
            // Delete selected transitions
            QList<QListWidgetItem*> selected = transitionsList->selectedItems();

            if (selected.isEmpty()) {
                QMessageBox::warning(&dialog, "No Selection", "Please select at least one transition to delete.");
                return;
            }

            int deletedCount = 0;
            for (auto* item : selected) {
                QString toState = item->data(Qt::UserRole).toString();
                QString symbolsStr = item->data(Qt::UserRole + 1).toString();

                // Handle multiple symbols (split by ", ")
                QStringList symbols = symbolsStr.split(", ");
                for (const QString& symbol : symbols) {
                    QString actualSymbol = symbol;
                    if (actualSymbol == "ε") {
                        actualSymbol = "E";
                    }

                    if (currentAutomaton->removeTransition(currentSelectedStateId, toState, actualSymbol)) {
                        deletedCount++;
                    }
                }
            }

            if (deletedCount > 0) {
                statusBar()->showMessage(QString("✓ Deleted %1 transition(s)").arg(deletedCount), 3000);
                updateProperties();
                if (canvas) {
                    canvas->update();
                }
                dialog.accept();
            }
        }
    });

    dialog.exec();
}

void MainWindow::onConvertNFAtoDFA() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

    if (currentAutomaton->isDFA()) {
        showStyledMessageBox("Info", "This automaton is already a DFA.", QMessageBox::Information);
        return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning",
                             "Current automaton is not valid. Please ensure it has an initial state.",
                             QMessageBox::Warning);
        return;
    }

    try {
        NFAtoDFA converter;
        Automaton* dfaAutomaton = converter.convert(currentAutomaton);

        if (dfaAutomaton) {
            QString id = generateAutomatonId();
            dfaAutomaton->setName(currentAutomaton->getName() + " (DFA)");

            int stateCount = dfaAutomaton->getStateCount();
            int cols = qCeil(qSqrt(stateCount));
            int row = 0, col = 0;

            for (auto& state : dfaAutomaton->getStates()) {
                state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
                col++;
                if (col >= cols) {
                    col = 0;
                    row++;
                }
            }

            automatons[id] = dfaAutomaton;
            updateAutomatonList();

            for (int i = 0; i < automatonList->count(); ++i) {
                QListWidgetItem* item = automatonList->item(i);
                if (item && item->data(Qt::UserRole).toString() == id) {
                    automatonList->setCurrentItem(item);
                    setCurrentAutomaton(dfaAutomaton);
                    break;
                }
            }

            showStyledMessageBox("Success",
                                 QString("NFA converted to DFA successfully!\n\n"
                                         "Original NFA states: %1\n"
                                         "Resulting DFA states: %2")
                                     .arg(currentAutomaton->getStateCount())
                                     .arg(dfaAutomaton->getStateCount()),
                                 QMessageBox::Information);

            statusBar()->showMessage("NFA converted to DFA");
        }
    } catch (const std::exception& e) {
        showStyledMessageBox("Error",
                             QString("Failed to convert NFA to DFA: %1").arg(e.what()),
                             QMessageBox::Critical);
    }
}

void MainWindow::onMinimizeDFA() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

    // Check if it's an NFA
    if (currentAutomaton->isNFA()) {
        showStyledMessageBox("Cannot Minimize NFA",
                             "DFA minimization can only be applied to DFAs.\n\n"
                             "This automaton is an NFA (Non-deterministic Finite Automaton).\n\n"
                             "💡 Tip: Convert it to a DFA first using 'Convert NFA → DFA' button, "
                             "then minimize the resulting DFA.",
                             QMessageBox::Warning);
        return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning",
                             "Current automaton is not valid. Please ensure it has an initial state.",
                             QMessageBox::Warning);
        return;
    }

    // Check if DFA is actually valid (no epsilon, no multiple transitions)
    bool isActuallyDFA = true;
    for (const auto& t : currentAutomaton->getTransitions()) {
        if (t.isEpsilonTransition()) {
            isActuallyDFA = false;
            break;
        }
    }

    if (isActuallyDFA) {
        for (const auto& state : currentAutomaton->getStates()) {
            QMap<QString, int> symbolCount;
            for (const auto& t : currentAutomaton->getTransitions()) {
                if (t.getFromStateId() == state.getId()) {
                    for (const auto& sym : t.getSymbols()) {
                        symbolCount[sym]++;
                        if (symbolCount[sym] > 1) {
                            isActuallyDFA = false;
                            break;
                        }
                    }
                }
                if (!isActuallyDFA) break;
            }
            if (!isActuallyDFA) break;
        }
    }

    if (!isActuallyDFA) {
        showStyledMessageBox("Invalid DFA",
                             "This automaton is marked as DFA but violates DFA rules!\n\n"
                             "• It may have epsilon transitions\n"
                             "• It may have multiple transitions per symbol from the same state\n\n"
                             "Please fix the DFA or convert from NFA properly.",
                             QMessageBox::Warning);
        return;
    }

    try {
        DFAMinimizer minimizer;
        Automaton* minimizedDFA = minimizer.minimize(currentAutomaton);

        if (minimizedDFA) {
            QString id = generateAutomatonId();
            minimizedDFA->setName(currentAutomaton->getName() + " (Minimized)");

            // Layout minimized states
            int stateCount = minimizedDFA->getStateCount();
            int cols = qCeil(qSqrt(stateCount));
            int row = 0, col = 0;

            for (auto& state : minimizedDFA->getStates()) {
                state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
                col++;
                if (col >= cols) {
                    col = 0;
                    row++;
                }
            }

            automatons[id] = minimizedDFA;
            updateAutomatonList();

            // Select the minimized DFA
            for (int i = 0; i < automatonList->count(); ++i) {
                QListWidgetItem* item = automatonList->item(i);
                if (item && item->data(Qt::UserRole).toString() == id) {
                    automatonList->setCurrentItem(item);
                    setCurrentAutomaton(minimizedDFA);
                    break;
                }
            }

            int originalStates = currentAutomaton->getStateCount();
            int minimizedStates = minimizedDFA->getStateCount();
            int reduction = originalStates - minimizedStates;

            QString resultMsg = QString(
                                    "DFA minimized successfully!\n\n"
                                    "Original states: %1\n"
                                    "Minimized states: %2\n"
                                    "States removed: %3\n\n"
                                    "The minimized DFA accepts the same language with fewer states."
                                    ).arg(originalStates).arg(minimizedStates).arg(reduction);

            showStyledMessageBox("Minimization Complete", resultMsg, QMessageBox::Information);

            statusBar()->showMessage(QString("DFA minimized: %1 → %2 states").arg(originalStates).arg(minimizedStates), 5000);
        } else {
            showStyledMessageBox("Error", "Failed to minimize DFA.", QMessageBox::Critical);
        }
    } catch (const std::exception& e) {
        showStyledMessageBox("Error",
                             QString("Failed to minimize DFA: %1").arg(e.what()),
                             QMessageBox::Critical);
    }
}

void MainWindow::onTestInput() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

    if (currentAutomaton->getStateCount() == 0) {
        showStyledMessageBox("Warning", "Automaton has no states.", QMessageBox::Warning);
        return;
    }

    if (currentAutomaton->getInitialStateId().isEmpty()) {
        showStyledMessageBox("Warning",
                             "No initial state defined.\n\nDouble-click a state and mark it as 'Initial State'.",
                             QMessageBox::Warning);
        return;
    }

    bool hasFinalState = false;
    for (const auto& state : currentAutomaton->getStates()) {
        if (state.getIsFinal()) {
            hasFinalState = true;
            break;
        }
    }

    if (!hasFinalState) {
        QMessageBox msgBox(this);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QLabel { color: black; }"
            "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
            );
        msgBox.setWindowTitle("No Final State");
        msgBox.setText("Automaton has no final/accepting states.\nNo input will be accepted.\n\nContinue anyway?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);

        if (msgBox.exec() == QMessageBox::No) return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning", "Current automaton is not valid.", QMessageBox::Warning);
        return;
    }

    QString input = testInputField->text();
    bool accepted = currentAutomaton->accepts(input);

    QString result = QString("<div style='color: black;'>");
    result += QString("<hr><b>Input:</b> \"%1\"<br>").arg(input.isEmpty() ? "(empty string)" : input);
    result += QString("<b>Result:</b> <span style='color: %1;'><b>%2</b></span><br>")
                  .arg(accepted ? "green" : "red")
                  .arg(accepted ? "✓ ACCEPTED" : "✗ REJECTED");
    result += QString("<b>Automaton:</b> %1 (%2)</div>")
                  .arg(currentAutomaton->getName())
                  .arg(currentAutomaton->isDFA() ? "DFA" : "NFA");

    if (testResultsText) {
        testResultsText->append(result);
        testResultsText->ensureCursorVisible();
    }

    statusBar()->showMessage(accepted ? "Input ACCEPTED ✓" : "Input REJECTED ✗");
}

void MainWindow::onClearTest() {
    if (testResultsText) {
        testResultsText->clear();
    }
    if (testInputField) {
        testInputField->clear();
    }
}

void MainWindow::onAutomatonModified() {
    updateProperties();
}

void MainWindow::onStateSelected(const QString& stateId) {
    currentSelectedStateId = stateId;
    updateProperties();
}

void MainWindow::onNew() {
    onNewAutomaton();
}

void MainWindow::onOpen() {
    showStyledMessageBox("Info", "Load functionality will be implemented in next phase.",
                         QMessageBox::Information);
}

void MainWindow::onSave() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton to save.", QMessageBox::Warning);
        return;
    }

    showStyledMessageBox("Info", "Save functionality will be implemented in next phase.",
                         QMessageBox::Information);
}

void MainWindow::onExit() {
    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; padding: 5px 15px; }"
        );
    msgBox.setWindowTitle("Exit");
    msgBox.setText("Are you sure you want to exit?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        qApp->quit();
    }
}

void MainWindow::onAbout() {
    showStyledMessageBox("About Compiler Project",
                         "<h2>Compiler Project - Automaton Designer</h2>"
                         "<p>Version 1.0</p>"
                         "<p>A visual tool for designing and simulating finite automatons.</p>"
                         "<p><b>Features:</b></p>"
                         "<ul>"
                         "<li>Create DFA and NFA automatons</li>"
                         "<li>Visual automaton designer</li>"
                         "<li>Convert NFA to DFA</li>"
                         "<li>Minimize DFA (remove redundant states)</li>"
                         "<li>Test input strings</li>"
                         "<li>Use 'E' for epsilon transitions</li>"
                         "<li>Delete states and transitions</li>"
                         "</ul>"
                         "<p><b>Algorithms Implemented:</b></p>"
                         "<ul>"
                         "<li>Subset Construction (NFA to DFA)</li>"
                         "<li>Table-Filling Algorithm (DFA Minimization)</li>"
                         "<li>Epsilon Closure</li>"
                         "</ul>"
                         "<p>Developed for Theory of Computation course.</p>",
                         QMessageBox::Information);
}

void MainWindow::updateProperties() {
    // Check if UI elements are initialized
    if (!typeLabel || !stateCountLabel || !transitionCountLabel ||
        !alphabetLabel || !transitionTable || !convertNFAtoDFABtn ||
        !minimizeDFABtn || !selectedStateLabel || !deleteStateBtn) {
        return;
    }

    if (!currentAutomaton) {
        typeLabel->setText("Type: N/A");
        stateCountLabel->setText("States: 0");
        transitionCountLabel->setText("Transitions: 0");
        alphabetLabel->setText("Alphabet: {}");
        transitionTable->setRowCount(0);
        convertNFAtoDFABtn->setEnabled(false);
        minimizeDFABtn->setEnabled(false);

        selectedStateLabel->setText("No state selected");
        selectedStateLabel->setStyleSheet(
            "background-color: #f5f5f5; color: #999; padding: 8px; "
            "border: 1px solid #ddd; border-radius: 3px;"
            );
        deleteStateBtn->setVisible(false);

        return;
    }

    // Determine actual type
    QString typeText;
    bool isActuallyDFA = true;

    for (const auto& t : currentAutomaton->getTransitions()) {
        if (t.isEpsilonTransition()) {
            isActuallyDFA = false;
            break;
        }
    }

    if (isActuallyDFA) {
        for (const auto& state : currentAutomaton->getStates()) {
            QMap<QString, int> symbolCount;
            for (const auto& t : currentAutomaton->getTransitions()) {
                if (t.getFromStateId() == state.getId()) {
                    for (const auto& sym : t.getSymbols()) {
                        symbolCount[sym]++;
                        if (symbolCount[sym] > 1) {
                            isActuallyDFA = false;
                            break;
                        }
                    }
                }
                if (!isActuallyDFA) break;
            }
            if (!isActuallyDFA) break;
        }
    }

    if (currentAutomaton->isDFA()) {
        if (isActuallyDFA) {
            typeText = "Type: <span style='color: #0078d7; font-weight: bold;'>DFA ✓</span>";
        } else {
            typeText = "Type: <span style='color: #dc3545; font-weight: bold;'>DFA ⚠ (Invalid)</span>";
        }
    } else {
        typeText = "Type: <span style='color: #28a745; font-weight: bold;'>NFA</span>";
    }

    typeLabel->setText(typeText);

    QString initialStateId = currentAutomaton->getInitialStateId();
    QString stateText = QString("States: %1").arg(currentAutomaton->getStateCount());
    if (initialStateId.isEmpty() && currentAutomaton->getStateCount() > 0) {
        stateText += " <span style='color: #dc3545;'>⚠</span>";
    }
    stateCountLabel->setText(stateText);

    transitionCountLabel->setText(QString("Transitions: %1")
                                      .arg(currentAutomaton->getTransitionCount()));

    QSet<QString> alphabet = currentAutomaton->getAlphabet();
    QStringList alphList = alphabet.values();
    alphList.sort();

    QString alphText = QString("Alphabet: {%1}").arg(alphList.join(", "));
    if (alphabet.isEmpty()) {
        alphText = "Alphabet: <span style='color: #999;'>{empty}</span>";
    }
    alphabetLabel->setText(alphText);

    // Update selected state display
    if (!currentSelectedStateId.isEmpty()) {
        const State* selectedState = currentAutomaton->getState(currentSelectedStateId);
        if (selectedState) {
            QString stateInfo = QString("Selected: <b>%1</b>").arg(selectedState->getLabel());
            if (selectedState->getIsInitial()) stateInfo += " [Initial]";
            if (selectedState->getIsFinal()) stateInfo += " [Final]";

            // Count transitions from this state
            int transCount = currentAutomaton->getTransitionsFrom(currentSelectedStateId).size();
            stateInfo += QString("<br><small>%1 outgoing transition(s)</small>").arg(transCount);

            selectedStateLabel->setText(stateInfo);
            selectedStateLabel->setStyleSheet(
                "background-color: #e3f2fd; color: #0d47a1; padding: 8px; "
                "border: 1px solid #90caf9; border-radius: 3px; font-weight: bold;"
                );
            deleteStateBtn->setVisible(true);
        } else {
            // State was deleted
            currentSelectedStateId = "";
            selectedStateLabel->setText("No state selected");
            selectedStateLabel->setStyleSheet(
                "background-color: #f5f5f5; color: #999; padding: 8px; "
                "border: 1px solid #ddd; border-radius: 3px;"
                );
            deleteStateBtn->setVisible(false);
        }
    } else {
        selectedStateLabel->setText("Click a state to see delete options");
        selectedStateLabel->setStyleSheet(
            "background-color: #fff3cd; color: #856404; padding: 8px; "
            "border: 1px solid #ffc107; border-radius: 3px; font-style: italic;"
            );
        deleteStateBtn->setVisible(false);
    }

    updateTransitionTable();

    // Enable/disable buttons based on automaton type and validity
    convertNFAtoDFABtn->setEnabled(currentAutomaton->isNFA() &&
                                   currentAutomaton->isValid());

    minimizeDFABtn->setEnabled(currentAutomaton->isDFA() &&
                               isActuallyDFA &&
                               currentAutomaton->isValid());
}

void MainWindow::updateTransitionTable() {
    if (!currentAutomaton || !transitionTable) return;

    const auto& transitions = currentAutomaton->getTransitions();
    transitionTable->setRowCount(transitions.size());

    int row = 0;
    for (const auto& trans : transitions) {
        transitionTable->setItem(row, 0,
                                 new QTableWidgetItem(trans.getFromStateId()));
        transitionTable->setItem(row, 1,
                                 new QTableWidgetItem(trans.getSymbolsString()));
        transitionTable->setItem(row, 2,
                                 new QTableWidgetItem(trans.getToStateId()));
        row++;
    }
}

void MainWindow::updateAutomatonList() {
    if (!automatonList) return;

    automatonList->clear();

    for (auto it = automatons.begin(); it != automatons.end(); ++it) {
        QString id = it.key();
        Automaton* automaton = it.value();

        QString displayName = automaton->getName();
        QString typeIndicator = automaton->isDFA() ? " 🔵" : " 🟢";

        QListWidgetItem* item = new QListWidgetItem(displayName + typeIndicator);
        item->setData(Qt::UserRole, id);

        QString tooltip = QString("%1\nType: %2\nStates: %3\nTransitions: %4")
                              .arg(automaton->getName())
                              .arg(automaton->isDFA() ? "DFA" : "NFA")
                              .arg(automaton->getStateCount())
                              .arg(automaton->getTransitionCount());
        item->setToolTip(tooltip);

        automatonList->addItem(item);
    }
}

QString MainWindow::generateAutomatonId() {
    return QString("auto_%1").arg(automatonCounter++);
}

void MainWindow::setCurrentAutomaton(Automaton* automaton) {
    currentAutomaton = automaton;
    currentSelectedStateId = "";
    if (canvas) {
        canvas->setAutomaton(automaton);
    }
    updateProperties();
}

void MainWindow::showStyledMessageBox(const QString& title, const QString& message,
                                      QMessageBox::Icon icon) {
    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; min-width: 300px; }"
        "QPushButton { color: black; background-color: #e0e0e0; border: 1px solid #999; "
        "padding: 5px 15px; min-width: 60px; }"
        "QPushButton:hover { background-color: #d0d0d0; }"
        );
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.exec();
}

void MainWindow::setupCentralTabs() {
    // Create tab widget
    centralTabs = new QTabWidget(this);
    if (!centralTabs) {
        qCritical() << "Failed to create central tabs!";
        return;
    }

    centralTabs->setTabPosition(QTabWidget::North);
    centralTabs->setMovable(false);

    // Tab 1: Automaton Designer
    automatonTab = new QWidget();
    QVBoxLayout* automatonLayout = new QVBoxLayout(automatonTab);
    automatonLayout->setContentsMargins(0, 0, 0, 0);

    // Create canvas here
    canvas = new AutomatonCanvas(automatonTab);
    if (!canvas) {
        qCritical() << "Failed to create AutomatonCanvas!";
        return;
    }
    automatonLayout->addWidget(canvas);

    centralTabs->addTab(automatonTab, "🤖 Automaton Designer");

    // Tab 2: Lexical Analyzer
    lexerWidget = new LexerWidget();
    if (!lexerWidget) {
        qCritical() << "Failed to create LexerWidget!";
        return;
    }

    // Set automaton manager AFTER creating widget - with null check
    if (automatonManager && lexerWidget) {
        lexerWidget->setAutomatonManager(automatonManager);
    } else {
        qWarning() << "AutomatonManager or LexerWidget is null, cannot set manager";
    }

    centralTabs->addTab(lexerWidget, "🔍 Lexical Analyzer");

    parserWidget = new ParserWidget();
    if (automatonManager && parserWidget) {
        parserWidget->setAutomatonManager(automatonManager);
    }
    centralTabs->addTab(parserWidget, "🌳 Parser & Parse Tree");

    semanticWidget = new SemanticAnalyzerWidget();
    if (automatonManager) semanticWidget->setAutomatonManager(automatonManager);
    centralTabs->addTab(semanticWidget, "🔬 Semantic Analysis");

    // Set as central widget
    setCentralWidget(centralTabs);

    // Connect tab changes
    connect(centralTabs, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);
}

void MainWindow::onTabChanged(int index) {
    // Hide all docks for non-automaton tabs
    bool showDocks = (index == 0);

    if (toolsDock) toolsDock->setVisible(showDocks);
    if (automatonListDock) automatonListDock->setVisible(showDocks);
    if (propertiesDock) propertiesDock->setVisible(showDocks);
    if (testingDock) testingDock->setVisible(showDocks);

    switch(index) {
    case 0:
        statusBar()->showMessage("Automaton Designer", 3000);
        break;
    case 1:
        statusBar()->showMessage("Lexical Analyzer", 3000);
        break;
    case 2:
        statusBar()->showMessage("Parser & Parse Tree", 3000);
        break;
    case 3:
        statusBar()->showMessage("Semantic Analysis & Translation", 3000);
        break;
    }
}
