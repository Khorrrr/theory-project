#ifndef PARSETREEWIDGET_H
#define PARSETREEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include "./src/models/Grammar/ParseTree.h"
#include <QMap>
#include <QPointF>

class ParseTreeCanvas : public QWidget {
    Q_OBJECT

private:
    std::shared_ptr<ParseTreeNode> root;
    QMap<ParseTreeNode*, QPointF> nodePositions;
    QMap<ParseTreeNode*, QRectF> nodeBounds;

    double nodeWidth;
    double nodeHeight;
    double horizontalSpacing;
    double verticalSpacing;

public:
    explicit ParseTreeCanvas(QWidget *parent = nullptr);

    void setParseTree(const ParseTree& tree);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void calculateLayout();
    double calculateSubtreeLayout(std::shared_ptr<ParseTreeNode> node, double x, double y, int depth);

    void drawNode(QPainter& painter, std::shared_ptr<ParseTreeNode> node);
    void drawEdge(QPainter& painter, const QPointF& from, const QPointF& to);

    void drawEdgesRecursive(QPainter& painter, std::shared_ptr<ParseTreeNode> node);
    void drawNodesRecursive(QPainter& painter, std::shared_ptr<ParseTreeNode> node);

    QRectF getNodeRect(const QPointF& pos) const;

    int getMaxDepth(std::shared_ptr<ParseTreeNode> node, int currentDepth = 0);
    int countLeaves(std::shared_ptr<ParseTreeNode> node);
};

class ParseTreeWidget : public QWidget {
    Q_OBJECT

private:
    ParseTreeCanvas* canvas;
    QScrollArea* scrollArea;

public:
    explicit ParseTreeWidget(QWidget *parent = nullptr);

    void setParseTree(const ParseTree& tree);
    void clear();
};

#endif // PARSETREEWIDGET_H
