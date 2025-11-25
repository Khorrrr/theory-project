#include "DFAMinimizer.h"
#include <QQueue>
#include <QDebug>
#include <algorithm>

DFAMinimizer::DFAMinimizer() {}

Automaton* DFAMinimizer::minimize(const Automaton* dfa) {
    if (!dfa || !dfa->isDFA() || !dfa->isValid()) {
        return nullptr;
    }

    // Create a working copy
    Automaton* workingDFA = new Automaton(dfa->getId(), dfa->getName(), AutomatonType::DFA);

    // Copy states
    for (const auto& state : dfa->getStates()) {
        workingDFA->addState(state);
    }

    // Copy transitions
    for (const auto& trans : dfa->getTransitions()) {
        workingDFA->addTransition(trans);
    }

    // Copy alphabet
    for (const auto& symbol : dfa->getAlphabet()) {
        workingDFA->addToAlphabet(symbol);
    }

    workingDFA->setInitialState(dfa->getInitialStateId());

    // Step 1: Remove unreachable states
    removeUnreachableStates(workingDFA);

    // Step 2: Find distinguishable state pairs
    QSet<QPair<QString, QString>> distinguishable = findDistinguishablePairs(workingDFA);

    // Step 3: Create equivalence classes
    QVector<QSet<QString>> equivalenceClasses = createEquivalenceClasses(workingDFA, distinguishable);

    // Step 4: Build minimized DFA
    Automaton* minimizedDFA = buildMinimizedDFA(workingDFA, equivalenceClasses);

    delete workingDFA;

    return minimizedDFA;
}

void DFAMinimizer::removeUnreachableStates(Automaton* dfa) {
    QSet<QString> reachable = getReachableStates(dfa);

    // Remove unreachable states
    QVector<State> statesToRemove;
    for (const auto& state : dfa->getStates()) {
        if (!reachable.contains(state.getId())) {
            statesToRemove.push_back(state);
        }
    }

    for (const auto& state : statesToRemove) {
        dfa->removeState(state.getId());
    }
}

QSet<QString> DFAMinimizer::getReachableStates(const Automaton* dfa) {
    QSet<QString> reachable;
    QQueue<QString> queue;

    QString initial = dfa->getInitialStateId();
    if (initial.isEmpty()) {
        return reachable;
    }

    reachable.insert(initial);
    queue.enqueue(initial);

    while (!queue.isEmpty()) {
        QString current = queue.dequeue();

        for (const auto& trans : dfa->getTransitionsFrom(current)) {
            QString next = trans.getToStateId();
            if (!reachable.contains(next)) {
                reachable.insert(next);
                queue.enqueue(next);
            }
        }
    }

    return reachable;
}

QSet<QPair<QString, QString>> DFAMinimizer::findDistinguishablePairs(const Automaton* dfa) {
    QSet<QPair<QString, QString>> distinguishable;
    QVector<QString> stateIds;

    for (const auto& state : dfa->getStates()) {
        stateIds.push_back(state.getId());
    }

    // Mark pairs where one is final and one is not (base case)
    for (int i = 0; i < stateIds.size(); i++) {
        for (int j = i + 1; j < stateIds.size(); j++) {
            const State* s1 = dfa->getState(stateIds[i]);
            const State* s2 = dfa->getState(stateIds[j]);

            if (s1->getIsFinal() != s2->getIsFinal()) {
                distinguishable.insert(makePair(stateIds[i], stateIds[j]));
            }
        }
    }

    // Iteratively mark distinguishable pairs
    bool changed = true;
    while (changed) {
        changed = false;

        for (int i = 0; i < stateIds.size(); i++) {
            for (int j = i + 1; j < stateIds.size(); j++) {
                QString s1 = stateIds[i];
                QString s2 = stateIds[j];

                auto pair = makePair(s1, s2);

                if (distinguishable.contains(pair)) {
                    continue; // Already marked
                }

                // Check if any symbol makes them distinguishable
                for (const auto& symbol : dfa->getAlphabet()) {
                    QString next1, next2;

                    // Find next state for s1 on symbol
                    for (const auto& trans : dfa->getTransitionsFrom(s1)) {
                        if (trans.hasSymbol(symbol)) {
                            next1 = trans.getToStateId();
                            break;
                        }
                    }

                    // Find next state for s2 on symbol
                    for (const auto& trans : dfa->getTransitionsFrom(s2)) {
                        if (trans.hasSymbol(symbol)) {
                            next2 = trans.getToStateId();
                            break;
                        }
                    }

                    // If both have transitions and lead to distinguishable states
                    if (!next1.isEmpty() && !next2.isEmpty() && next1 != next2) {
                        auto nextPair = makePair(next1, next2);
                        if (distinguishable.contains(nextPair)) {
                            distinguishable.insert(pair);
                            changed = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    return distinguishable;
}

QVector<QSet<QString>> DFAMinimizer::createEquivalenceClasses(
    const Automaton* dfa,
    const QSet<QPair<QString, QString>>& distinguishable) {

    QVector<QSet<QString>> classes;
    QSet<QString> processed;

    for (const auto& state : dfa->getStates()) {
        QString stateId = state.getId();

        if (processed.contains(stateId)) {
            continue;
        }

        // Create new equivalence class
        QSet<QString> equivalenceClass;
        equivalenceClass.insert(stateId);
        processed.insert(stateId);

        // Find all states equivalent to this one
        for (const auto& other : dfa->getStates()) {
            QString otherId = other.getId();

            if (processed.contains(otherId) || stateId == otherId) {
                continue;
            }

            // Check if they're distinguishable
            auto pair = makePair(stateId, otherId);
            if (!distinguishable.contains(pair)) {
                // They're equivalent
                equivalenceClass.insert(otherId);
                processed.insert(otherId);
            }
        }

        classes.push_back(equivalenceClass);
    }

    return classes;
}

Automaton* DFAMinimizer::buildMinimizedDFA(
    const Automaton* dfa,
    const QVector<QSet<QString>>& equivalenceClasses) {

    Automaton* minimized = new Automaton("", dfa->getName() + " (Minimized)", AutomatonType::DFA);

    // Copy alphabet
    for (const auto& symbol : dfa->getAlphabet()) {
        minimized->addToAlphabet(symbol);
    }

    // Create states for each equivalence class
    QMap<int, QString> classToStateId;

    for (int i = 0; i < equivalenceClasses.size(); i++) {
        const QSet<QString>& eqClass = equivalenceClasses[i];

        // Use representative (first state in sorted order)
        QStringList sorted = eqClass.values();
        sorted.sort();
        QString representative = sorted.first();

        // Create new state name
        QString newStateId;
        if (eqClass.size() == 1) {
            newStateId = representative;
        } else {
            newStateId = "{" + sorted.join(",") + "}";
        }

        classToStateId[i] = newStateId;

        // Determine if this class contains initial or final state
        bool isInitial = eqClass.contains(dfa->getInitialStateId());
        bool isFinal = false;

        for (const auto& stateId : eqClass) {
            const State* state = dfa->getState(stateId);
            if (state && state->getIsFinal()) {
                isFinal = true;
                break;
            }
        }

        State newState(newStateId, newStateId, QPointF(0, 0));
        newState.setIsInitial(isInitial);
        newState.setIsFinal(isFinal);
        minimized->addState(newState);

        if (isInitial) {
            minimized->setInitialState(newStateId);
        }
    }

    // Create transitions
    for (int i = 0; i < equivalenceClasses.size(); i++) {
        const QSet<QString>& eqClass = equivalenceClasses[i];
        QString fromStateId = classToStateId[i];

        // Get representative
        QString representative = eqClass.values().first();

        // For each symbol, find where the representative goes
        for (const auto& symbol : dfa->getAlphabet()) {
            for (const auto& trans : dfa->getTransitionsFrom(representative)) {
                if (trans.hasSymbol(symbol)) {
                    // Find which class the target state belongs to
                    int targetClass = findClassIndex(equivalenceClasses, trans.getToStateId());
                    if (targetClass >= 0) {
                        QString toStateId = classToStateId[targetClass];

                        Transition newTrans(fromStateId, toStateId, symbol);
                        minimized->addTransition(newTrans);
                    }
                    break; // DFA has only one transition per symbol
                }
            }
        }
    }

    return minimized;
}

int DFAMinimizer::findClassIndex(const QVector<QSet<QString>>& classes, const QString& stateId) {
    for (int i = 0; i < classes.size(); i++) {
        if (classes[i].contains(stateId)) {
            return i;
        }
    }
    return -1;
}

QPair<QString, QString> DFAMinimizer::makePair(const QString& s1, const QString& s2) {
    // Ensure consistent ordering
    if (s1 < s2) {
        return qMakePair(s1, s2);
    } else {
        return qMakePair(s2, s1);
    }
}
