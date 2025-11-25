#ifndef NFATODFA_H
#define NFATODFA_H

#include "./src/models/Automaton/Automaton.h"
#include <QSet>
#include <QMap>
#include <QString>

class NFAtoDFA {
public:
    NFAtoDFA();

    Automaton* convert(const Automaton* nfa);

private:
    QString setToString(const QSet<QString>& stateSet);
    QSet<QString> move(const Automaton* nfa, const QSet<QString>& states,
                       const QString& symbol);
};

#endif // NFATODFA_H
