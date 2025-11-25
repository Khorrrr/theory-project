#include "Production.h"
#include <QRegularExpression>

Production::Production() : nonTerminal("") {}

Production::Production(const QString& lhs, const QVector<QString>& rhs)
    : nonTerminal(lhs), symbols(rhs) {}

QString Production::toString() const {
    QString result = nonTerminal + " → ";

    if (symbols.isEmpty() || isEpsilon()) {
        result += "ε";
    } else {
        result += symbols.join(" ");
    }

    return result;
}

bool Production::isEmpty() const {
    return symbols.isEmpty();
}

bool Production::isEpsilon() const {
    return symbols.size() == 1 &&
           (symbols[0] == "ε" || symbols[0] == "epsilon" || symbols[0].isEmpty());
}

Production Production::fromString(const QString& str) {
    // Parse "E → E + T" or "E -> E + T"
    QString cleaned = str.trimmed();

    // Split by → or ->
    QStringList parts;
    if (cleaned.contains("→")) {
        parts = cleaned.split("→");
    } else if (cleaned.contains("->")) {
        parts = cleaned.split("->");
    } else {
        return Production();
    }

    if (parts.size() != 2) {
        return Production();
    }

    QString lhs = parts[0].trimmed();
    QString rhs = parts[1].trimmed();

    QVector<QString> symbols;

    if (rhs == "ε" || rhs == "epsilon" || rhs.isEmpty()) {
        symbols.append("ε");
    } else {
        // Use QRegularExpression instead of QRegExp
        QRegularExpression re("\\s+");
        QStringList rhsParts = rhs.split(re);
        for (const QString& part : rhsParts) {
            if (!part.trimmed().isEmpty()) {
                symbols.append(part.trimmed());
            }
        }
    }

    return Production(lhs, symbols);
}
