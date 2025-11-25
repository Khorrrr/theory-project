#ifndef PARSETREE_H
#define PARSETREE_H

#include <QString>
#include <QVector>
#include <memory>

class ParseTreeNode {
private:
    QString symbol;
    QString value;
    QVector<std::shared_ptr<ParseTreeNode>> children;
    bool isTerminal;

public:
    ParseTreeNode(const QString& sym, bool terminal = false);
    ParseTreeNode(const QString& sym, const QString& val, bool terminal = false);

    QString getSymbol() const { return symbol; }
    QString getValue() const { return value; }
    bool getIsTerminal() const { return isTerminal; }
    QVector<std::shared_ptr<ParseTreeNode>>& getChildren() { return children; }
    const QVector<std::shared_ptr<ParseTreeNode>>& getChildren() const { return children; }

    void setSymbol(const QString& s) { symbol = s; }
    void setValue(const QString& v) { value = v; }

    void addChild(std::shared_ptr<ParseTreeNode> child);

    QString toString(int indent = 0) const;
};

class ParseTree {
private:
    std::shared_ptr<ParseTreeNode> root;
    QString grammarName;

public:
    ParseTree();
    ParseTree(const QString& grammar);

    std::shared_ptr<ParseTreeNode> getRoot() const { return root; }
    QString getGrammarName() const { return grammarName; }

    void setRoot(std::shared_ptr<ParseTreeNode> r) { root = r; }
    void setGrammarName(const QString& name) { grammarName = name; }

    QString toString() const;
    bool isEmpty() const { return root == nullptr; }
};

#endif // PARSETREE_H
