#include "SymbolTable.h"

QString Symbol::getTypeString() const {
    return SymbolTable::typeToString(type);
}

QString Symbol::toString() const {
    QString result = QString("%1 : %2").arg(name).arg(getTypeString());
    if (isInitialized && !value.isEmpty()) {
        result += QString(" = %1").arg(value);
    }
    if (isConstant) {
        result += " [const]";
    }
    return result;
}

SymbolTable::SymbolTable() : currentScope(0) {
    scopes.append(QMap<QString, Symbol>());
}

void SymbolTable::enterScope() {
    currentScope++;
    if (currentScope >= scopes.size()) {
        scopes.append(QMap<QString, Symbol>());
    }
}

void SymbolTable::exitScope() {
    if (currentScope > 0) {
        scopes[currentScope].clear();
        currentScope--;
    }
}

bool SymbolTable::addSymbol(const Symbol& symbol) {
    if (existsInCurrentScope(symbol.name)) {
        return false;
    }

    Symbol newSymbol = symbol;
    newSymbol.scope = currentScope;
    scopes[currentScope][symbol.name] = newSymbol;
    allDiscoveredSymbols.append(newSymbol);

    return true;
}

bool SymbolTable::updateSymbol(const QString& name, const QString& value) {
    Symbol* sym = lookup(name);
    if (sym) {
        sym->value = value;
        sym->isInitialized = true;

        for(auto& discoveredSym : allDiscoveredSymbols) {
            if(discoveredSym.name == name && discoveredSym.scope == sym->scope) {
                discoveredSym.value = value;
                discoveredSym.isInitialized = true;
                break;
            }
        }
        return true;
    }
    return false;
}

Symbol* SymbolTable::lookup(const QString& name) {
    for (int i = currentScope; i >= 0; --i) {
        if (scopes[i].contains(name)) {
            return &scopes[i][name];
        }
    }
    return nullptr;
}

const Symbol* SymbolTable::lookup(const QString& name) const {
    for (int i = currentScope; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &(it.value());  // Use it.value() instead of it->second
        }
    }
    return nullptr;
}

bool SymbolTable::exists(const QString& name) const {
    return lookup(name) != nullptr;
}

bool SymbolTable::existsInCurrentScope(const QString& name) const {
    return scopes[currentScope].contains(name);
}

QVector<Symbol> SymbolTable::getSymbolsInScope(int scope) const {
    QVector<Symbol> result;
    if (scope >= 0 && scope < scopes.size()) {
        for (const auto& symbol : scopes[scope]) {
            result.append(symbol);
        }
    }
    return result;
}

void SymbolTable::clear() {
    scopes.clear();
    scopes.append(QMap<QString, Symbol>());
    currentScope = 0;
    allDiscoveredSymbols.clear();
}

QString SymbolTable::toString() const {
    QString result = "Symbol Table:\n\n";

    for (int i = 0; i <= currentScope; ++i) {
        result += QString("Scope %1:\n").arg(i);
        if (scopes[i].isEmpty()) {
            result += "  (empty)\n";
        } else {
            for (const auto& symbol : scopes[i]) {
                result += QString("  %1\n").arg(symbol.toString());
            }
        }
        result += "\n";
    }

    return result;
}

SymbolType SymbolTable::stringToType(const QString& typeStr) {
    QString lower = typeStr.toLower();

    if (lower == "int" || lower == "integer") return SymbolType::INTEGER;
    if (lower == "float") return SymbolType::FLOAT;
    if (lower == "double") return SymbolType::DOUBLE;
    if (lower == "char") return SymbolType::CHAR;
    if (lower == "string") return SymbolType::STRING;
    if (lower == "bool" || lower == "boolean") return SymbolType::BOOLEAN;
    if (lower == "void") return SymbolType::VOID;

    return SymbolType::UNKNOWN;
}

QString SymbolTable::typeToString(SymbolType type) {
    switch (type) {
    case SymbolType::INTEGER: return "int";
    case SymbolType::FLOAT: return "float";
    case SymbolType::DOUBLE: return "double";
    case SymbolType::CHAR: return "char";
    case SymbolType::STRING: return "string";
    case SymbolType::BOOLEAN: return "bool";
    case SymbolType::VOID: return "void";
    case SymbolType::FUNCTION: return "function";
    default: return "unknown";
    }
}
