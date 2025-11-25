cmake_minimum_required(VERSION 3.16)
project(TheoryProject VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define source files
set(MAIN_SOURCES
    main.cpp
)

# UI sources
set(UI_SOURCES
    ui/Semantic/SemanticAnalyzerWidget.cpp
    ui/Semantic/SemanticAnalyzerWidget.h
)

# Utility sources
set(UTILS_SOURCES
    src/utils/LexicalAnalysis/Lexer.cpp
    src/utils/LexicalAnalysis/Lexer.h
    src/utils/LexicalAnalysis/AutomatonManager.cpp
    src/utils/LexicalAnalysis/AutomatonManager.h
    src/utils/Semantic/SemanticAnalyzer.cpp
    src/utils/Semantic/SemanticAnalyzer.h
    src/utils/Semantic/CodeGenerator.cpp
    src/utils/Semantic/CodeGenerator.h
    utils/Semantic/MLTranslationBridge.cpp
    utils/Semantic/MLTranslationBridge.h
)

# Model sources
set(MODEL_SOURCES
    models/LexicalAnalysis/Token.cpp
    models/LexicalAnalysis/Token.h
    models/Semantic/SymbolTable.cpp
    models/Semantic/SymbolTable.h
    models/Automaton/State.cpp
    models/Automaton/State.h
    models/Automaton/Transition.cpp
    models/Automaton/Transition.h
    models/Automaton/Automaton.cpp
    models/Automaton/Automaton.h
)

# Combine all sources
set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${UI_SOURCES}
    ${UTILS_SOURCES}
    ${MODEL_SOURCES}
)

# Create executable
add_executable(theory-project ${ALL_SOURCES})

# Link Qt libraries
target_link_libraries(theory-project
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Include directories
target_include_directories(theory-project PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/LexicalAnalysis
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/Semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/models/LexicalAnalysis
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Automaton
)

# Compiler-specific options
if(MSVC)
    target_compile_options(theory-project PRIVATE /W4)
else()
    target_compile_options(theory-project PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(theory-project PRIVATE DEBUG)
endif()

# Installation rules
install(TARGETS theory-project
    RUNTIME DESTINATION bin
)

# Custom targets for ML server
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ml_translator")
    # Add ML server startup script
    add_custom_target(start_ml_server
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ml_translator/start_server.py --check-only
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ml_translator
        COMMENT "Checking ML server dependencies"
    )

    # Install Python requirements (optional)
    find_program(Python3_EXECUTABLE python3)
    if(Python3_EXECUTABLE)
        add_custom_target(install_ml_requirements
            COMMAND ${Python3_EXECUTABLE} -m pip install -r requirements.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ml_translator
            COMMENT "Installing ML translation dependencies"
        )
    endif()
endif()

# Documentation
add_custom_target(docs
    COMMAND echo "Documentation target - add doxygen support here"
    COMMENT "Generating documentation"
)

# Testing
enable_testing()
add_subdirectory(tests OPTIONAL)

# Package configuration
include(GNUInstallDirs)
include(CPack)

set(CPACK_PACKAGE_NAME "TheoryProject")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Theory of Computation Project with ML Code Translation")
set(CPACK_PACKAGE_VENDOR "TheoryProject Team")

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;RPM")
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Theory Project Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt version: ${Qt6_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  - Semantic Analysis: YES")
message(STATUS "  - Code Translation: YES")
message(STATUS "  - ML Translation: YES")
message(STATUS "  - Qt Network: YES")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ml_translator")
    message(STATUS "  - Python ML Server: YES")
else()
    message(STATUS "  - Python ML Server: NO")
endif()
message(STATUS "")