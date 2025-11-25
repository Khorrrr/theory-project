#include "Transition.h"
#include <QStringList>

Transition::Transition()
    : fromStateId(""), toStateId("") {}

Transition::Transition(const QString& from, const QString& to, const QString& symbol)
    : fromStateId(from), toStateId(to) {
    if (!symbol.isEmpty()) {
        symbols.insert(symbol);
    }
}

bool Transition::isEpsilonTransition() const {
    return symbols.contains("E") ||
           symbols.contains("ε") ||
           symbols.contains("epsilon") ||
           symbols.contains("");
}

QString Transition::getSymbolsString() const {
    QStringList list;
    for (const auto& sym : symbols) {
        if (sym == "E") {
            list.append("ε"); // Display as ε but internally stored as E
        } else {
            list.append(sym);
        }
    }
    list.sort();
    return list.join(", ");
}

bool Transition::hasSymbol(const QString& symbol) const {
    // Check for epsilon equivalents
    if (symbol == "E" || symbol == "ε" || symbol == "epsilon" || symbol.isEmpty()) {
        return symbols.contains("E") ||
               symbols.contains("ε") ||
               symbols.contains("epsilon") ||
               symbols.contains("");
    }
    return symbols.contains(symbol);
}
