#include "NFAtoDFA.h"
#include <QQueue>
#include <QDebug>

NFAtoDFA::NFAtoDFA() {}

Automaton* NFAtoDFA::convert(const Automaton* nfa) {
    if (!nfa || !nfa->isValid()) {
        return nullptr;
    }

    Automaton* dfa = new Automaton("", "", AutomatonType::DFA);
    dfa->setName(nfa->getName() + " (DFA)");

    QSet<QString> alphabet = nfa->getAlphabet();
    for (const auto& symbol : alphabet) {
        dfa->addToAlphabet(symbol);
    }

    QSet<QString> initialNFAStates;
    initialNFAStates.insert(nfa->getInitialStateId());
    QSet<QString> initialDFAState = nfa->epsilonClosure(initialNFAStates);

    QMap<QString, QSet<QString>> dfaStateMap;
    QQueue<QSet<QString>> unmarkedStates;

    QString initialStateId = setToString(initialDFAState);
    dfaStateMap[initialStateId] = initialDFAState;
    unmarkedStates.enqueue(initialDFAState);

    bool initialIsFinal = false;
    for (const auto& nfaStateId : initialDFAState) {
        const State* nfaState = nfa->getState(nfaStateId);
        if (nfaState && nfaState->getIsFinal()) {
            initialIsFinal = true;
            break;
        }
    }

    State initialState(initialStateId, initialStateId, QPointF(100, 100));
    initialState.setIsInitial(true);
    initialState.setIsFinal(initialIsFinal);
    dfa->addState(initialState);
    dfa->setInitialState(initialStateId);

    while (!unmarkedStates.isEmpty()) {
        QSet<QString> currentSet = unmarkedStates.dequeue();
        QString currentId = setToString(currentSet);

        for (const auto& symbol : alphabet) {
            QSet<QString> nextSet = move(nfa, currentSet, symbol);
            nextSet = nfa->epsilonClosure(nextSet);

            if (nextSet.isEmpty()) {
                continue;
            }

            QString nextId = setToString(nextSet);

            if (!dfaStateMap.contains(nextId)) {
                dfaStateMap[nextId] = nextSet;
                unmarkedStates.enqueue(nextSet);

                bool isFinal = false;
                for (const auto& nfaStateId : nextSet) {
                    const State* nfaState = nfa->getState(nfaStateId);
                    if (nfaState && nfaState->getIsFinal()) {
                        isFinal = true;
                        break;
                    }
                }

                State newState(nextId, nextId, QPointF(0, 0));
                newState.setIsFinal(isFinal);
                dfa->addState(newState);
            }

            Transition trans(currentId, nextId, symbol);
            dfa->addTransition(trans);
        }
    }

    dfa->setType(AutomatonType::DFA);
    return dfa;
}

QString NFAtoDFA::setToString(const QSet<QString>& stateSet) {
    if (stateSet.isEmpty()) {
        return "∅";
    }

    QStringList list = stateSet.values();
    list.sort();
    return "{" + list.join(",") + "}";
}

QSet<QString> NFAtoDFA::move(const Automaton* nfa, const QSet<QString>& states,
                             const QString& symbol) {
    QSet<QString> result;

    for (const auto& stateId : states) {
        const auto& transitions = nfa->getTransitionsFrom(stateId);

        for (const auto& trans : transitions) {
            if (trans.hasSymbol(symbol)) {
                result.insert(trans.getToStateId());
            }
        }
    }

    return result;
}
