#ifndef DFAMINIMIZER_H
#define DFAMINIMIZER_H

#include "./src/models/Automaton/Automaton.h"
#include <QSet>
#include <QMap>
#include <QPair>
#include <QVector>

class DFAMinimizer {
public:
    DFAMinimizer();

    // Minimize a DFA using table-filling algorithm
    Automaton* minimize(const Automaton* dfa);

private:
    // Remove unreachable states
    void removeUnreachableStates(Automaton* dfa);

    // Get reachable states from initial state
    QSet<QString> getReachableStates(const Automaton* dfa);

    // Table-filling algorithm to find distinguishable states
    QSet<QPair<QString, QString>> findDistinguishablePairs(const Automaton* dfa);

    // Create equivalence classes from distinguishable pairs
    QVector<QSet<QString>> createEquivalenceClasses(
        const Automaton* dfa,
        const QSet<QPair<QString, QString>>& distinguishable
        );

    // Build minimized DFA from equivalence classes
    Automaton* buildMinimizedDFA(
        const Automaton* dfa,
        const QVector<QSet<QString>>& equivalenceClasses
        );

    // Find which equivalence class contains a state
    int findClassIndex(const QVector<QSet<QString>>& classes, const QString& stateId);

    // Create canonical pair (order doesn't matter for distinguishability)
    QPair<QString, QString> makePair(const QString& s1, const QString& s2);
};

#endif // DFAMINIMIZER_H
