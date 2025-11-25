#include "AutomatonManager.h"
#include <QDebug>

AutomatonManager::AutomatonManager() {
    createDefaultAutomatons();
}

AutomatonManager::~AutomatonManager() {
    clear();
}

bool AutomatonManager::addAutomaton(const Automaton& automaton) {
    if (exists(automaton.getId())) {
        return false;
    }
    idToIndex[automaton.getId()] = automatons.size();
    automatons.push_back(automaton);
    return true;
}

bool AutomatonManager::removeAutomaton(const QString& id) {
    int index = getAutomatonIndex(id);
    if (index == -1) return false;

    automatons.removeAt(index);
    idToIndex.clear();

    for (int i = 0; i < automatons.size(); ++i) {
        idToIndex[automatons[i].getId()] = i;
    }
    return true;
}

Automaton* AutomatonManager::getAutomaton(const QString& id) {
    int index = getAutomatonIndex(id);
    return (index != -1) ? &automatons[index] : nullptr;
}

const Automaton* AutomatonManager::getAutomaton(const QString& id) const {
    int index = getAutomatonIndex(id);
    return (index != -1) ? &automatons[index] : nullptr;
}

int AutomatonManager::getAutomatonIndex(const QString& id) const {
    return idToIndex.value(id, -1);
}

QVector<QString> AutomatonManager::getAllIds() const {
    QVector<QString> ids;
    for (const auto& automaton : automatons) {
        ids.push_back(automaton.getId());
    }
    return ids;
}

bool AutomatonManager::exists(const QString& id) const {
    return idToIndex.contains(id);
}

void AutomatonManager::clear() {
    automatons.clear();
    idToIndex.clear();
}

QString AutomatonManager::findMatchingAutomaton(const QString& input) const {
    for (const auto& automaton : automatons) {
        if (automaton.accepts(input)) {
            return automaton.getId();
        }
    }
    return "";
}

QVector<QString> AutomatonManager::findAllMatchingAutomatons(const QString& input) const {
    QVector<QString> matches;
    for (const auto& automaton : automatons) {
        if (automaton.accepts(input)) {
            matches.push_back(automaton.getId());
        }
    }
    return matches;
}

void AutomatonManager::createDefaultAutomatons() {
    createIdentifierAutomaton();
    createIntegerAutomaton();
    createFloatAutomaton();
}

void AutomatonManager::createIdentifierAutomaton() {
    Automaton automaton("IDENTIFIER", "Identifier", AutomatonType::DFA);

    State s0("q0", "q0", QPointF(100, 100));
    s0.setIsInitial(true);
    State s1("q1", "q1", QPointF(200, 100));
    s1.setIsFinal(true);

    automaton.addState(s0);
    automaton.addState(s1);

    for (char c = 'a'; c <= 'z'; ++c) {
        automaton.addTransition(Transition("q0", "q1", QString(c)));
        automaton.addTransition(Transition("q1", "q1", QString(c)));
    }
    for (char c = 'A'; c <= 'Z'; ++c) {
        automaton.addTransition(Transition("q0", "q1", QString(c)));
        automaton.addTransition(Transition("q1", "q1", QString(c)));
    }
    automaton.addTransition(Transition("q0", "q1", "_"));
    automaton.addTransition(Transition("q1", "q1", "_"));

    for (char c = '0'; c <= '9'; ++c) {
        automaton.addTransition(Transition("q1", "q1", QString(c)));
    }

    addAutomaton(automaton);
}

void AutomatonManager::createIntegerAutomaton() {
    Automaton automaton("INTEGER", "Integer", AutomatonType::DFA);

    State s0("q0", "q0", QPointF(100, 200));
    s0.setIsInitial(true);
    State s1("q1", "q1", QPointF(200, 200));
    s1.setIsFinal(true);

    automaton.addState(s0);
    automaton.addState(s1);

    for (char c = '0'; c <= '9'; ++c) {
        automaton.addTransition(Transition("q0", "q1", QString(c)));
        automaton.addTransition(Transition("q1", "q1", QString(c)));
    }

    addAutomaton(automaton);
}

void AutomatonManager::createFloatAutomaton() {
    Automaton automaton("FLOAT", "Float", AutomatonType::DFA);

    State s0("q0", "q0", QPointF(100, 300));
    s0.setIsInitial(true);
    State s1("q1", "q1", QPointF(200, 300));
    State s2("q2", "q2", QPointF(300, 300));
    State s3("q3", "q3", QPointF(400, 300));
    s3.setIsFinal(true);

    automaton.addState(s0);
    automaton.addState(s1);
    automaton.addState(s2);
    automaton.addState(s3);

    for (char c = '0'; c <= '9'; ++c) {
        automaton.addTransition(Transition("q0", "q1", QString(c)));
        automaton.addTransition(Transition("q1", "q1", QString(c)));
        automaton.addTransition(Transition("q2", "q3", QString(c)));
        automaton.addTransition(Transition("q3", "q3", QString(c)));
    }

    automaton.addTransition(Transition("q1", "q2", "."));

    addAutomaton(automaton);
}
