#include <QApplication> // Required for creating a Qt application object.
#include "MainWindow.h" // Includes the declaration of our main application window.

// The main entry point of the application.
// argc: The number of command-line arguments.
// argv: An array of command-line argument strings.
int main(int argc, char *argv[]) {
    // Creates a QApplication instance. This object manages the GUI application's
    // control flow and main settings. It must be created before any other
    // GUI-related objects.
    QApplication app(argc, argv);

    // Sets the application's visible name, which might be used in window titles
    // or system menus.
    app.setApplicationName("Compiler Project");
    // Sets the application's version string.
    app.setApplicationVersion("1.0");
    // Sets the name of the organization that created the application.
    // This is often used for persistent application settings storage.
    app.setOrganizationName("University");

    // Creates an instance of our custom MainWindow class.
    // This is the primary user interface window for the application.
    MainWindow window;
    // Displays the main window on the screen. The window will not be visible
    // until show() is called.
    window.show();

    // Enters the Qt event loop. This function hands over control to Qt,
    // which then processes events (like user clicks, key presses, window resizes)
    // and dispatches them to the relevant widgets. The application remains
    // running until exit() is called.
    return app.exec();
}
