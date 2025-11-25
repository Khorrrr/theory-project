#include "ParseTree.h"

ParseTreeNode::ParseTreeNode(const QString& sym, bool terminal)
    : symbol(sym), value(sym), isTerminal(terminal) {}

ParseTreeNode::ParseTreeNode(const QString& sym, const QString& val, bool terminal)
    : symbol(sym), value(val), isTerminal(terminal) {}

void ParseTreeNode::addChild(std::shared_ptr<ParseTreeNode> child) {
    children.append(child);
}

QString ParseTreeNode::toString(int indent) const {
    QString result;
    QString indentStr = QString(indent * 2, ' ');

    if (isTerminal) {
        result += indentStr + "Terminal: " + symbol;
        if (value != symbol) {
            result += " (" + value + ")";
        }
        result += "\n";
    } else {
        result += indentStr + "NonTerminal: " + symbol + "\n";
        for (const auto& child : children) {
            result += child->toString(indent + 1);
        }
    }

    return result;
}

ParseTree::ParseTree() : grammarName("") {}

ParseTree::ParseTree(const QString& grammar) : grammarName(grammar) {}

QString ParseTree::toString() const {
    if (!root) {
        return "Empty parse tree";
    }

    QString result = "Parse Tree for: " + grammarName + "\n\n";
    result += root->toString();

    return result;
}
