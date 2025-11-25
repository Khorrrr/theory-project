# Build Instructions for CompilerProject

## Prerequisites
- Qt 5.15 or Qt 6.x with the following modules:
  - Qt Widgets
  - Qt Network
  - Qt Core
  - C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)

## Build Steps

### Using qmake (Recommended)
1. **Install Qt and qmake:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt6-base-dev qt6-tools-dev cmake build-essential

   # Or for Qt5
   sudo apt-get install qt5-default qttools5-dev-tools build-essential
   ```

2. **Generate Makefile:**
   ```bash
   cd theory-project
   qmake CompilerProject.pro
   ```

3. **Build the project:**
   ```bash
   make -j$(nproc)  # Use all available CPU cores
   ```

4. **Run the application:**
   ```bash
   ./CompilerProject
   ```

### Using Qt Creator (GUI)
1. Open Qt Creator
2. File → Open Project → Select `CompilerProject.pro`
3. Configure the kit (Qt version, compiler)
4. Build → Build Project
5. Run → Run

## ML Translation Features
The project includes Python-based ML translation features:

### Setup ML Translator (Optional)
```bash
cd src/ml_translator
pip install -r requirements.txt
python app.py  # Start ML translation server
```

### ML Files Structure
- `src/ml_translator/` - Python ML translation server
- `src/utils/Semantic/MLTranslationBridge.cpp` - C++ bridge to ML server
- All ML files are included in the build via `OTHER_FILES` in the .pro file

## Project Structure
```
theory-project/
├── CompilerProject.pro          # qmake project file
├── BUILD_INSTRUCTIONS.md       # This file
└── src/
    ├── main.cpp               # Application entry point
    ├── models/                # Data model classes
    │   ├── Automaton/         # Finite automaton models
    │   ├── Grammar/           # Grammar models
    │   ├── LexicalAnalysis/   # Token models
    │   └── Semantic/          # Symbol table models
    ├── ui/                    # Qt user interface
    │   ├── MainWindow.cpp/.h  # Main application window
    │   ├── Automaton/         # Automaton visualization
    │   ├── Grammar/           # Grammar parsing UI
    │   ├── LexicalAnalysis/   # Lexer UI
    │   └── Semantic/          # Semantic analyzer UI
    ├── utils/                 # Utility classes
    │   ├── Automaton/         # NFA-to-DFA, minimization
    │   ├── Grammar/           # Parser implementation
    │   ├── LexicalAnalysis/   # Lexer implementation
    │   └── Semantic/          # Semantic analysis, code generation
    └── ml_translator/         # Python ML translation server
```

## Configuration
- `CONFIG += c++17` - Uses C++17 standard
- `QT += core gui widgets network` - Required Qt modules
- `DEFINES += SOURCE_PATH=\\\"$$PWD\\\"` - Source path available at compile time

## Troubleshooting
- **Missing Qt modules:** Make sure all required Qt modules are installed
- **Compiler errors:** Verify C++17 support in your compiler
- **Link errors:** Check Qt library paths and linker configuration
- **ML translation:** Optional - works without Python dependencies

## Legacy Build System Removed
The project has been migrated from CMake to qmake. `CMakeLists.txt` has been removed and replaced with `CompilerProject.pro`.