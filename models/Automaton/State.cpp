#include "State.h"
#include <cmath>

State::State()
    : id(""), label(""), position(0, 0),
    isInitial(false), isFinal(false), radius(30.0) {}

State::State(const QString& id, const QString& label, const QPointF& pos)
    : id(id), label(label.isEmpty() ? id : label), position(pos),
    isInitial(false), isFinal(false), radius(30.0) {}

bool State::containsPoint(const QPointF& point) const {
    double dx = point.x() - position.x();
    double dy = point.y() - position.y();
    return (dx * dx + dy * dy) <= (radius * radius);
}
