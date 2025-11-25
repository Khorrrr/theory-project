#ifndef AUTOMATONMANAGER_H
#define AUTOMATONMANAGER_H

#include "./src/models/Automaton/Automaton.h"
#include <QVector>
#include <QMap>

class AutomatonManager {
private:
    QVector<Automaton> automatons;
    QMap<QString, int> idToIndex;

public:
    AutomatonManager();
    ~AutomatonManager();

    bool addAutomaton(const Automaton& automaton);
    bool removeAutomaton(const QString& id);
    Automaton* getAutomaton(const QString& id);
    const Automaton* getAutomaton(const QString& id) const;
    int getAutomatonIndex(const QString& id) const;

    QVector<Automaton>& getAutomatons() { return automatons; }
    const QVector<Automaton>& getAutomatons() const { return automatons; }
    int getCount() const { return automatons.size(); }
    void clear();

    QVector<QString> getAllIds() const;
    bool exists(const QString& id) const;

    QString findMatchingAutomaton(const QString& input) const;
    QVector<QString> findAllMatchingAutomatons(const QString& input) const;

    void createDefaultAutomatons();
    void createIdentifierAutomaton();
    void createIntegerAutomaton();
    void createFloatAutomaton();
};

#endif // AUTOMATONMANAGER_H
