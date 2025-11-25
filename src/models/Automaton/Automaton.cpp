#include "Automaton.h"
#include <QQueue>
#include <QDebug>
#include <algorithm>

Automaton::Automaton()
    : id(""), name("Untitled"), type(AutomatonType::NFA), initialStateId("") {}

Automaton::Automaton(const QString& id, const QString& name, AutomatonType type)
    : id(id), name(name), type(type), initialStateId("") {}

// State Management
bool Automaton::addState(const State& state) {
    for (const auto& s : states) {
        if (s.getId() == state.getId()) {
            return false;
        }
    }
    states.push_back(state);

    if (state.getIsInitial()) {
        initialStateId = state.getId();
    }

    return true;
}

bool Automaton::removeState(const QString& stateId) {
    transitions.erase(
        std::remove_if(transitions.begin(), transitions.end(),
                       [&stateId](const Transition& t) {
                           return t.getFromStateId() == stateId ||
                                  t.getToStateId() == stateId;
                       }),
        transitions.end()
        );

    auto it = std::find_if(states.begin(), states.end(),
                           [&stateId](const State& s) { return s.getId() == stateId; });

    if (it != states.end()) {
        if (initialStateId == stateId) {
            initialStateId = "";
        }
        states.erase(it);
        return true;
    }

    return false;
}

State* Automaton::getState(const QString& stateId) {
    for (auto& state : states) {
        if (state.getId() == stateId) {
            return &state;
        }
    }
    return nullptr;
}

const State* Automaton::getState(const QString& stateId) const {
    for (const auto& state : states) {
        if (state.getId() == stateId) {
            return &state;
        }
    }
    return nullptr;
}

// Transition Management
bool Automaton::canAddTransition(const Transition& transition, QString* errorMsg) const {
    if (!getState(transition.getFromStateId()) ||
        !getState(transition.getToStateId())) {
        if (errorMsg) *errorMsg = "Source or destination state does not exist.";
        return false;
    }

    if (type == AutomatonType::DFA) {
        // Check for epsilon transitions
        if (transition.isEpsilonTransition()) {
            if (errorMsg) {
                *errorMsg = QString(
                    "❌ Cannot add epsilon (E) transition to a DFA!\n\n"
                    "This automaton is defined as a DFA (Deterministic Finite Automaton).\n"
                    "DFAs cannot have epsilon transitions.\n\n"
                    "💡 Solution: Create an NFA instead if you need epsilon transitions."
                    );
            }
            return false;
        }

        // Check for multiple transitions with same symbol
        for (const auto& t : transitions) {
            if (t.getFromStateId() == transition.getFromStateId()) {
                for (const auto& newSym : transition.getSymbols()) {
                    if (t.hasSymbol(newSym)) {
                        if (errorMsg) {
                            *errorMsg = QString(
                                            "❌ DFA Violation!\n\n"
                                            "State '%1' already has a transition on symbol '%2' going to state '%3'.\n\n"
                                            "In a DFA, each state can have only ONE transition per symbol.\n\n"
                                            "💡 Solution: Create an NFA if you need multiple transitions per symbol."
                                            )
                                            .arg(transition.getFromStateId())
                                            .arg(newSym)
                                            .arg(t.getToStateId());
                        }
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool Automaton::addTransition(const Transition& transition) {
    QString errorMsg;
    if (!canAddTransition(transition, &errorMsg)) {
        qDebug() << "Cannot add transition:" << errorMsg;
        return false;
    }

    for (auto& t : transitions) {
        if (t.getFromStateId() == transition.getFromStateId() &&
            t.getToStateId() == transition.getToStateId()) {
            for (const auto& sym : transition.getSymbols()) {
                t.addSymbol(sym);
                // Only add non-epsilon symbols to alphabet
                if (sym != "E" && sym != "ε" && sym != "epsilon" && !sym.isEmpty()) {
                    alphabet.insert(sym);
                }
            }
            return true;
        }
    }

    transitions.push_back(transition);

    // Only add non-epsilon symbols to alphabet
    for (const auto& sym : transition.getSymbols()) {
        if (sym != "E" && sym != "ε" && sym != "epsilon" && !sym.isEmpty()) {
            alphabet.insert(sym);
        }
    }

    return true;
}

bool Automaton::removeTransition(const QString& from, const QString& to, const QString& symbol) {
    for (auto it = transitions.begin(); it != transitions.end(); ++it) {
        if (it->getFromStateId() == from && it->getToStateId() == to) {
            if (symbol.isEmpty()) {
                transitions.erase(it);
                return true;
            } else {
                it->removeSymbol(symbol);
                if (it->getSymbols().isEmpty()) {
                    transitions.erase(it);
                }
                return true;
            }
        }
    }
    return false;
}

QVector<Transition> Automaton::getTransitionsFrom(const QString& stateId) const {
    QVector<Transition> result;
    for (const auto& t : transitions) {
        if (t.getFromStateId() == stateId) {
            result.push_back(t);
        }
    }
    return result;
}

void Automaton::setInitialState(const QString& stateId) {
    for (auto& state : states) {
        state.setIsInitial(false);
    }

    State* state = getState(stateId);
    if (state) {
        state->setIsInitial(true);
        initialStateId = stateId;
    }
}

void Automaton::addToAlphabet(const QString& symbol) {
    if (symbol != "E" && symbol != "ε" && symbol != "epsilon" && !symbol.isEmpty()) {
        alphabet.insert(symbol);
    }
}

// Validation
bool Automaton::isValid() const {
    if (states.isEmpty() || initialStateId.isEmpty()) {
        return false;
    }

    bool hasInitial = false;
    for (const auto& state : states) {
        if (state.getId() == initialStateId) {
            hasInitial = true;
            break;
        }
    }

    return hasInitial;
}

bool Automaton::isDFA() const {
    return type == AutomatonType::DFA;
}

bool Automaton::isNFA() const {
    return type == AutomatonType::NFA;
}

void Automaton::detectType() {
    for (const auto& t : transitions) {
        if (t.isEpsilonTransition()) {
            type = AutomatonType::NFA;
            return;
        }
    }

    for (const auto& state : states) {
        QMap<QString, int> symbolCount;
        for (const auto& t : transitions) {
            if (t.getFromStateId() == state.getId()) {
                for (const auto& sym : t.getSymbols()) {
                    symbolCount[sym]++;
                    if (symbolCount[sym] > 1) {
                        type = AutomatonType::NFA;
                        return;
                    }
                }
            }
        }
    }

    type = AutomatonType::DFA;
}

// Epsilon Closure
QSet<QString> Automaton::epsilonClosureHelper(const QString& stateId) const {
    QSet<QString> closure;
    QQueue<QString> queue;

    closure.insert(stateId);
    queue.enqueue(stateId);

    while (!queue.isEmpty()) {
        QString current = queue.dequeue();

        for (const auto& t : transitions) {
            if (t.getFromStateId() == current && t.isEpsilonTransition()) {
                if (!closure.contains(t.getToStateId())) {
                    closure.insert(t.getToStateId());
                    queue.enqueue(t.getToStateId());
                }
            }
        }
    }

    return closure;
}

QSet<QString> Automaton::epsilonClosure(const QSet<QString>& stateIds) const {
    QSet<QString> closure;
    for (const auto& id : stateIds) {
        closure.unite(epsilonClosureHelper(id));
    }
    return closure;
}

// Simulation
bool Automaton::accepts(const QString& input) const {
    if (!isValid()) {
        return false;
    }

    if (type == AutomatonType::DFA) {
        return acceptsDFA(input);
    } else {
        return acceptsNFA(input);
    }
}

bool Automaton::acceptsDFA(const QString& input) const {
    QString currentState = initialStateId;

    for (const QChar& ch : input) {
        QString symbol = QString(ch);
        bool found = false;

        for (const auto& t : transitions) {
            if (t.getFromStateId() == currentState && t.hasSymbol(symbol)) {
                currentState = t.getToStateId();
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    const State* state = getState(currentState);
    return state && state->getIsFinal();
}

bool Automaton::acceptsNFA(const QString& input) const {
    QSet<QString> currentStates;
    currentStates.insert(initialStateId);
    currentStates = epsilonClosure(currentStates);

    for (const QChar& ch : input) {
        QString symbol = QString(ch);
        QSet<QString> nextStates;

        for (const auto& stateId : currentStates) {
            for (const auto& t : transitions) {
                if (t.getFromStateId() == stateId && t.hasSymbol(symbol)) {
                    nextStates.insert(t.getToStateId());
                }
            }
        }

        currentStates = epsilonClosure(nextStates);

        if (currentStates.isEmpty()) {
            return false;
        }
    }

    for (const auto& stateId : currentStates) {
        const State* state = getState(stateId);
        if (state && state->getIsFinal()) {
            return true;
        }
    }

    return false;
}

void Automaton::clear() {
    states.clear();
    transitions.clear();
    alphabet.clear();
    initialStateId = "";
}
