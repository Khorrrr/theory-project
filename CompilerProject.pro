QT += core gui widgets network

CONFIG += c++17

DEFINES += SOURCE_PATH=\\\"$$PWD\\\"

TARGET = CompilerProject
TEMPLATE = app

SRCDIR = $$PWD/src

SOURCES += \
    $$SRCDIR/main.cpp \
    $$SRCDIR/models/Automaton/State.cpp \
    $$SRCDIR/models/Automaton/Transition.cpp \
    $$SRCDIR/models/Automaton/Automaton.cpp \
    $$SRCDIR/models/Grammar/Grammar.cpp \
    $$SRCDIR/ui/MainWindow.cpp \
    $$SRCDIR/ui/Automaton/AutomatonCanvas.cpp \
    $$SRCDIR/utils/Automaton/NFAtoDFA.cpp \
    $$SRCDIR/utils/Automaton/DFAMinimizer.cpp \
    $$SRCDIR/utils/Grammar/Parser.cpp \
    $$SRCDIR/utils/Semantic/SemanticAnalyzer.cpp \
    $$SRCDIR/utils/Semantic/CodeGenerator.cpp \
    $$SRCDIR/models/Grammar/ParseTree.cpp \
    $$SRCDIR/models/Grammar/Production.cpp \
    $$SRCDIR/models/LexicalAnalysis/Token.cpp \
    $$SRCDIR/models/Semantic/SymbolTable.cpp \
    $$SRCDIR/ui/Grammar/ParseTreeWidget.cpp \
    $$SRCDIR/ui/Grammar/ParserWidget.cpp \
    $$SRCDIR/ui/LexicalAnalysis/LexerWidget.cpp \
    $$SRCDIR/ui/Semantic/SemanticAnalyzerWidget.cpp \
    $$SRCDIR/utils/LexicalAnalysis/AutomatonManager.cpp \
    $$SRCDIR/utils/LexicalAnalysis/Lexer.cpp \
    $$SRCDIR/utils/Semantic/MLTranslationBridge.cpp

HEADERS += \
    $$SRCDIR/models/Automaton/State.h \
    $$SRCDIR/models/Automaton/Transition.h \
    $$SRCDIR/models/Automaton/Automaton.h \
    $$SRCDIR/models/Grammar/Grammar.h \
    $$SRCDIR/ui/MainWindow.h \
    $$SRCDIR/ui/Automaton/AutomatonCanvas.h \
    $$SRCDIR/utils/Automaton/NFAtoDFA.h \
    $$SRCDIR/utils/Automaton/DFAMinimizer.h \
    $$SRCDIR/utils/Semantic/SemanticAnalyzer.h \
    $$SRCDIR/utils/Semantic/CodeGenerator.h \
    $$SRCDIR/utils/Grammar/Parser.h \
    $$SRCDIR/models/Grammar/ParseTree.h \
    $$SRCDIR/models/Grammar/Production.h \
    $$SRCDIR/models/LexicalAnalysis/Token.h \
    $$SRCDIR/models/Semantic/SymbolTable.h \
    $$SRCDIR/ui/Grammar/ParseTreeWidget.h \
    $$SRCDIR/ui/Grammar/ParserWidget.h \
    $$SRCDIR/ui/LexicalAnalysis/LexerWidget.h \
    $$SRCDIR/ui/Semantic/SemanticAnalyzerWidget.h \
    $$SRCDIR/utils/LexicalAnalysis/AutomatonManager.h \
    $$SRCDIR/utils/LexicalAnalysis/Lexer.h \
    $$SRCDIR/utils/Semantic/MLTranslationBridge.h

INCLUDEPATH += \
    $$SRCDIR \
    $$SRCDIR/models \
    $$SRCDIR/ui \
    $$SRCDIR/utils \
    $$SRCDIR/ml_translator


OTHER_FILES += \
    $$SRCDIR/ml_translator/__init__.py \
    $$SRCDIR/ml_translator/app.py \
    $$SRCDIR/ml_translator/config.py \
    $$SRCDIR/ml_translator/requirements.txt \
    $$SRCDIR/ml_translator/start_server.py \
    $$SRCDIR/ml_translator/models/__init__.py \
    $$SRCDIR/ml_translator/models/base_model.py \
    $$SRCDIR/ml_translator/models/codegen_model.py \
    $$SRCDIR/ml_translator/postprocessing/__init__.py \
    $$SRCDIR/ml_translator/postprocessing/formatter.py \
    $$SRCDIR/ml_translator/postprocessing/validator.py \
    $$SRCDIR/ml_translator/preprocessing/__init__.py \
    $$SRCDIR/ml_translator/preprocessing/ast_processor.py \
    $$SRCDIR/ml_translator/preprocessing/tokenizer.py