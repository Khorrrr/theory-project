#ifndef AUTOMATON_H
#define AUTOMATON_H

#include "State.h"
#include "Transition.h"
#include <QVector>
#include <QMap>
#include <QString>
#include <QSet>

enum class AutomatonType {
    DFA,
    NFA
};

class Automaton {
private:
    QString id;
    QString name;
    AutomatonType type;
    QVector<State> states;
    QVector<Transition> transitions;
    QSet<QString> alphabet;
    QString initialStateId;

public:
    Automaton();
    Automaton(const QString& id, const QString& name, AutomatonType type);

    // State management
    bool addState(const State& state);
    bool removeState(const QString& stateId);
    State* getState(const QString& stateId);
    const State* getState(const QString& stateId) const;
    QVector<State>& getStates() { return states; }
    const QVector<State>& getStates() const { return states; }

    // Transition management
    bool addTransition(const Transition& transition);
    bool canAddTransition(const Transition& transition, QString* errorMsg = nullptr) const;
    bool removeTransition(const QString& from, const QString& to, const QString& symbol);
    QVector<Transition> getTransitionsFrom(const QString& stateId) const;
    QVector<Transition>& getTransitions() { return transitions; }
    const QVector<Transition>& getTransitions() const { return transitions; }

    // Automaton properties
    QString getId() const { return id; }
    QString getName() const { return name; }
    AutomatonType getType() const { return type; }
    void setType(AutomatonType t) { type = t; }
    void setName(const QString& n) { name = n; }

    QString getInitialStateId() const { return initialStateId; }
    void setInitialState(const QString& stateId);

    QSet<QString> getAlphabet() const { return alphabet; }
    void addToAlphabet(const QString& symbol);

    // Validation
    bool isValid() const;
    bool isDFA() const;
    bool isNFA() const;

    // Simulation
    bool accepts(const QString& input) const;
    QSet<QString> epsilonClosure(const QSet<QString>& stateIds) const;

    // Conversion
    void detectType();

    // Utility
    void clear();
    int getStateCount() const { return states.size(); }
    int getTransitionCount() const { return transitions.size(); }

private:
    QSet<QString> epsilonClosureHelper(const QString& stateId) const;
    bool acceptsNFA(const QString& input) const;
    bool acceptsDFA(const QString& input) const;
};

#endif // AUTOMATON_H
