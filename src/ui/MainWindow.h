#ifndef MAINWINDOW_H // Prevents multiple inclusions of this header file.
#define MAINWINDOW_H

// Qt GUI module includes for core window functionalities and UI elements.
#include <QMainWindow>   // Base class for main application windows.
#include <QToolBar>      // For creating toolbars (rows of buttons).
#include <QMenuBar>      // For creating the application's menu bar.
#include <QStatusBar>    // For displaying status messages at the bottom of the window.
#include <QDockWidget>   // For creating dockable windows that can be moved around.
#include <QListWidget>   // For displaying a list of items.
#include <QTextEdit>     // For multi-line text editing.
#include <QPushButton>   // For push buttons.
#include <QLineEdit>     // For single-line text editing.
#include <QLabel>        // For displaying text or images.
#include <QVBoxLayout>   // For vertical arrangement of widgets.
#include <QHBoxLayout>   // For horizontal arrangement of widgets.
#include <QGroupBox>     // For grouping related widgets with a title.
#include <QRadioButton>  // For radio buttons (exclusive selection).
#include <QTableWidget>  // For displaying data in a table format.
#include <QMap>          // For storing key-value pairs (like a dictionary).
#include <QMessageBox>   // For displaying standard message boxes.
#include <QTabWidget>    // For creating a tabbed interface.

// Project-specific includes for various UI components and data models.
#include "./src/ui/Automaton/AutomatonCanvas.h"          // Custom widget for drawing automatons.
#include "./src/models/Automaton/Automaton.h"            // Data model for an automaton.
#include "./src/ui/LexicalAnalysis/LexerWidget.h"        // Widget for lexical analysis features.
#include "./src/utils/LexicalAnalysis/AutomatonManager.h" // Manages a collection of automatons.
#include "./src/ui/Grammar/ParserWidget.h"                // Widget for parsing grammar.
#include "./src/ui/Semantic/SemanticAnalyzerWidget.h"    // Widget for semantic analysis.

/**
 * @brief The MainWindow class serves as the main application window for the Compiler Project.
 *
 * It orchestrates all the major UI components, handles user interactions, and manages
 * the different analysis and generation tools (Automaton, Lexer, Parser, Semantic Analyzer).
 */
class MainWindow : public QMainWindow {
    Q_OBJECT // Macro that enables Qt's meta-object system features for this class,
             // such as signals, slots, and introspection.

private:
    // --- Central Tab Widgets ---
    QTabWidget* centralTabs;          // Main tab widget to switch between different tools (Automaton, Lexer, Parser, Semantic).
    QWidget* automatonTab;            // The base widget for the Automaton editor tab.
    LexerWidget* lexerWidget;         // Widget instance for the lexical analysis tool.
    AutomatonManager* automatonManager; // Manages the collection of NFA/DFA automatons.
    ParserWidget* parserWidget;        // Widget instance for the parsing tool.
    SemanticAnalyzerWidget* semanticWidget; // Widget instance for the semantic analysis and code generation tool.

    // --- Automaton Canvas ---
    AutomatonCanvas* canvas;          // Custom drawing area for visualizing automatons.

    // --- Automaton Data Management ---
    QMap<QString, Automaton*> automatons; // Stores all created automatons, mapped by their unique IDs.
    Automaton* currentAutomaton;           // Pointer to the currently active automaton being displayed/edited.
    int automatonCounter;                  // Counter used to generate unique IDs for new automatons.

    // --- Dock Widgets ---
    QDockWidget* toolsDock;           // Dock for mode selection (select, add state, add transition, delete).
    QDockWidget* automatonListDock;   // Dock for listing and managing created automatons.
    QDockWidget* propertiesDock;      // Dock for displaying and editing properties of selected states/transitions.
    QDockWidget* testingDock;         // Dock for testing automaton input strings.

    // --- Tools Dock Widgets ---
    QRadioButton* selectModeBtn;       // Button to activate selection mode on the canvas.
    QRadioButton* addStateModeBtn;     // Button to activate add state mode on the canvas.
    QRadioButton* addTransitionModeBtn; // Button to activate add transition mode on the canvas.
    QRadioButton* deleteModeBtn;       // Button to activate delete mode on the canvas.
    QPushButton* clearCanvasBtn;       // Button to clear all elements from the current automaton canvas.

    // --- Automaton List Dock Widgets ---
    QListWidget* automatonList;        // List display of all managed automatons.
    QPushButton* newAutomatonBtn;      // Button to create a new automaton.
    QPushButton* deleteAutomatonBtn;   // Button to delete the selected automaton.
    QPushButton* renameAutomatonBtn;   // Button to rename the selected automaton.

    // --- Properties Dock Widgets ---
    QLabel* typeLabel;                 // Displays the type of the selected automaton (NFA/DFA).
    QLabel* stateCountLabel;           // Displays the number of states in the current automaton.
    QLabel* transitionCountLabel;      // Displays the number of transitions in the current automaton.
    QLabel* alphabetLabel;             // Displays the alphabet used by the current automaton.
    QTableWidget* transitionTable;     // Table for displaying transitions of the current automaton.
    QPushButton* convertNFAtoDFABtn;   // Button to convert the current NFA to a DFA.
    QPushButton* minimizeDFABtn;       // Button to minimize the current DFA.

    QLabel* selectedStateLabel;        // Displays the ID of the currently selected state.
    QPushButton* deleteStateBtn;       // Button to delete the currently selected state or transition.

    QString currentSelectedStateId;    // Stores the ID of the state currently selected on the canvas.

    // --- Testing Dock Widgets ---
    QLineEdit* testInputField;         // Input field for entering strings to test against the automaton.
    QPushButton* testInputBtn;         // Button to initiate the test of the input string.
    QTextEdit* testResultsText;        // Displays the results of automaton tests.
    QPushButton* clearTestBtn;         // Button to clear the test input and results.

    // --- Menu Actions ---
    QAction* newAction;                // Action for creating a new project/file.
    QAction* openAction;               // Action for opening an existing project/file.
    QAction* saveAction;               // Action for saving the current project/file.
    QAction* exitAction;               // Action for exiting the application.
    QAction* aboutAction;              // Action for displaying information about the application.

    QAction* selectAction;             // Action to set canvas mode to selection.
    QAction* addStateAction;           // Action to set canvas mode to add state.
    QAction* addTransitionAction;      // Action to set canvas mode to add transition.
    QAction* deleteAction;             // Action to set canvas mode to delete.
    QAction* convertAction;            // Action to convert NFA to DFA.
    QAction* minimizeAction;           // Action to minimize DFA.

public:
    /**
     * @brief Constructor for the MainWindow.
     * @param parent The parent widget, typically nullptr for the main window.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor for the MainWindow.
     * Cleans up allocated resources.
     */
    ~MainWindow();

private slots:
    // --- Canvas Mode Handlers ---
    void onSelectMode();             // Slot triggered when select mode radio button is activated.
    void onAddStateMode();           // Slot triggered when add state mode radio button is activated.
    void onAddTransitionMode();      // Slot triggered when add transition mode radio button is activated.
    void onDeleteMode();             // Slot triggered when delete mode radio button is activated.

    // --- Automaton Management Handlers ---
    void onNewAutomaton();           // Slot to handle creation of a new automaton.
    void onDeleteAutomaton();        // Slot to handle deletion of the selected automaton.
    void onRenameAutomaton();        // Slot to handle renaming of the selected automaton.
    void onAutomatonSelected(QListWidgetItem* item); // Slot triggered when an automaton is selected in the list.
    void onClearCanvas();            // Slot to handle clearing the automaton canvas.

    // --- Automaton Conversion Handlers ---
    void onConvertNFAtoDFA();        // Slot to handle conversion of NFA to DFA.
    void onMinimizeDFA();            // Slot to handle minimization of DFA.

    // --- Automaton Testing Handlers ---
    void onTestInput();              // Slot to handle testing an input string against the current automaton.
    void onClearTest();              // Slot to handle clearing the test input and results.

    // --- Automaton Canvas Interaction Handlers ---
    void onAutomatonModified();      // Slot triggered when the current automaton data changes (e.g., state/transition added/removed).
    void onStateSelected(const QString& stateId); // Slot triggered when a state is selected on the canvas.
    void onDeleteStateOrTransition(); // Slot to handle deletion of a selected state or transition.

    // --- Menu Action Handlers ---
    void onNew();                    // Slot for the "New" menu action.
    void onOpen();                   // Slot for the "Open" menu action.
    void onSave();                   // Slot for the "Save" menu action.
    void onExit();                   // Slot for the "Exit" menu action.
    void onAbout();                  // Slot for the "About" menu action.

    // --- Tab Change Handler ---
    void onTabChanged(int index);    // Slot triggered when the active tab in centralTabs changes.

private:
    // --- UI Initialization Methods ---
    void createMenus();              // Sets up the application's menu bar.
    void createToolbar();            // Sets up the main toolbar (currently empty in the provided code).
    void createDockWidgets();        // Initializes and arranges all dockable widgets.
    void createToolsPanel();         // Creates the content for the tools dock.
    void createAutomatonListPanel(); // Creates the content for the automaton list dock.
    void createPropertiesPanel();    // Creates the content for the properties dock.
    void createTestingPanel();       // Creates the content for the testing dock.
    void setupCentralTabs();         // Sets up the central tab widget with different analysis tools.

    // --- UI Update Methods ---
    void updateProperties();         // Updates the properties dock with information about the current automaton.
    void updateTransitionTable();    // Updates the transition table in the properties dock.
    void updateAutomatonList();      // Refreshes the list of automatons in the automaton list dock.

    // --- Helper Methods ---
    QString generateAutomatonId();   // Generates a unique ID for a new automaton.
    void setCurrentAutomaton(Automaton* automaton); // Sets the currently active automaton and updates UI accordingly.

    /**
     * @brief Displays a styled QMessageBox with custom title, message, and icon.
     * @param title The title of the message box.
     * @param message The message content.
     * @param icon The icon to display (e.g., QMessageBox::Information, QMessageBox::Warning).
     */
    void showStyledMessageBox(const QString& title, const QString& message,
                              QMessageBox::Icon icon = QMessageBox::Information);
};

#endif // MAINWINDOW_H
