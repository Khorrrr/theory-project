#include "ParseTreeWidget.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>
#include <QDebug>

ParseTreeCanvas::ParseTreeCanvas(QWidget *parent)
    : QWidget(parent), nodeWidth(100), nodeHeight(50),
    horizontalSpacing(30), verticalSpacing(80) {

    setMinimumSize(800, 600);
    setStyleSheet("background-color: #1e1e1e;");
}

void ParseTreeCanvas::setParseTree(const ParseTree& tree) {
    root = tree.getRoot();
    nodePositions.clear();
    nodeBounds.clear();

    if (root) {
        // Calculate layout starting from left margin
        double startX = 50;  // Left margin
        double startY = 50;  // Top margin

        double treeWidth = calculateSubtreeLayout(root, startX, startY, 0);

        // Set widget size based on actual tree dimensions
        int maxDepth = getMaxDepth(root);

        int width = qMax(1000, (int)(treeWidth + 100));  // Add right margin
        int height = qMax(600, maxDepth * (int)(nodeHeight + verticalSpacing) + 150);

        setMinimumSize(width, height);
        resize(width, height);

        qDebug() << "Tree dimensions - Width:" << width << "Height:" << height
                 << "Max depth:" << maxDepth;
    }

    update();
}

void ParseTreeCanvas::clear() {
    root = nullptr;
    nodePositions.clear();
    nodeBounds.clear();
    update();
}

double ParseTreeCanvas::calculateSubtreeLayout(std::shared_ptr<ParseTreeNode> node,
                                               double x, double y, int depth) {
    if (!node) return x;

    const auto& children = node->getChildren();

    if (children.isEmpty()) {
        // Leaf node - position it at the current x
        QPointF pos(x, y);
        nodePositions[node.get()] = pos;
        nodeBounds[node.get()] = getNodeRect(pos);

        // Return the next available x position
        return x + nodeWidth + horizontalSpacing;
    }

    // Internal node - calculate positions for all children first
    double childY = y + nodeHeight + verticalSpacing;
    double childStartX = x;
    double leftmostChildX = x;
    double rightmostChildX = x;

    QVector<QPointF> childPositions;

    // Layout all children and track their positions
    for (int i = 0; i < children.size(); ++i) {
        double childTreeWidth = calculateSubtreeLayout(children[i], childStartX, childY, depth + 1);

        // Get the actual center position of this child
        QPointF childPos = nodePositions[children[i].get()];
        childPositions.append(childPos);

        if (i == 0) {
            leftmostChildX = childPos.x();
        }
        rightmostChildX = childPos.x();

        // Move to next position
        childStartX = childTreeWidth;
    }

    // Position parent centered above its children
    double parentX = (leftmostChildX + rightmostChildX) / 2.0;
    QPointF parentPos(parentX, y);

    nodePositions[node.get()] = parentPos;
    nodeBounds[node.get()] = getNodeRect(parentPos);

    // Return the rightmost edge of this subtree
    return childStartX;
}

void ParseTreeCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!root) {
        // Draw empty state message
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter,
                         "No parse tree to display\n\nParse some input to see the tree");
        return;
    }

    // Draw edges first (so they appear behind nodes)
    drawEdgesRecursive(painter, root);

    // Draw nodes on top
    drawNodesRecursive(painter, root);
}

void ParseTreeCanvas::drawEdgesRecursive(QPainter& painter, std::shared_ptr<ParseTreeNode> node) {
    if (!node || !nodePositions.contains(node.get())) return;

    QPointF parentPos = nodePositions[node.get()];
    QPointF parentCenter(parentPos.x() + nodeWidth / 2, parentPos.y() + nodeHeight);

    for (const auto& child : node->getChildren()) {
        if (nodePositions.contains(child.get())) {
            QPointF childPos = nodePositions[child.get()];
            QPointF childCenter(childPos.x() + nodeWidth / 2, childPos.y());
            drawEdge(painter, parentCenter, childCenter);

            // Recursively draw edges for children
            drawEdgesRecursive(painter, child);
        }
    }
}

void ParseTreeCanvas::drawNodesRecursive(QPainter& painter, std::shared_ptr<ParseTreeNode> node) {
    if (!node) return;

    drawNode(painter, node);

    for (const auto& child : node->getChildren()) {
        drawNodesRecursive(painter, child);
    }
}

void ParseTreeCanvas::drawNode(QPainter& painter, std::shared_ptr<ParseTreeNode> node) {
    if (!nodePositions.contains(node.get())) return;

    QPointF pos = nodePositions[node.get()];
    QRectF rect = getNodeRect(pos);

    // Choose colors based on node type
    QColor bgColor, borderColor, textColor;

    if (node->getIsTerminal()) {
        // Terminal: green
        bgColor = QColor(50, 150, 50);
        borderColor = QColor(100, 200, 100);
        textColor = Qt::white;
    } else if (node->getSymbol() == "ε") {
        // Epsilon: gray
        bgColor = QColor(80, 80, 80);
        borderColor = QColor(120, 120, 120);
        textColor = Qt::lightGray;
    } else {
        // Non-terminal: blue
        bgColor = QColor(30, 100, 150);
        borderColor = QColor(80, 150, 200);
        textColor = Qt::white;
    }

    // Draw node
    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(QBrush(bgColor));

    if (node->getIsTerminal()) {
        painter.drawRect(rect);
    } else {
        painter.drawEllipse(rect);
    }

    // Draw text
    painter.setPen(textColor);
    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true);
    painter.setFont(font);

    QString displayText = node->getSymbol();
    if (node->getValue() != node->getSymbol() && !node->getValue().isEmpty()) {
        displayText = node->getValue();
    }

    // Handle long text
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(displayText, Qt::ElideRight, (int)nodeWidth - 10);

    painter.drawText(rect, Qt::AlignCenter, elidedText);
}

void ParseTreeCanvas::drawEdge(QPainter& painter, const QPointF& from, const QPointF& to) {
    painter.setPen(QPen(QColor(150, 150, 150), 2));
    painter.drawLine(from, to);

    // Optional: Draw arrow head
    QPen arrowPen(QColor(150, 150, 150), 2);
    painter.setPen(arrowPen);

    // Calculate arrow direction
    double angle = atan2(to.y() - from.y(), to.x() - from.x());
    double arrowSize = 8;

    QPointF arrowP1 = to - QPointF(sin(angle + M_PI / 3) * arrowSize,
                                   cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = to - QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                   cos(angle + M_PI - M_PI / 3) * arrowSize);

    QVector<QPointF> arrowHead;
    arrowHead << to << arrowP1 << arrowP2;

    painter.setBrush(QColor(150, 150, 150));
    painter.drawPolygon(arrowHead.data(), 3);
}

QRectF ParseTreeCanvas::getNodeRect(const QPointF& pos) const {
    return QRectF(pos.x(), pos.y(), nodeWidth, nodeHeight);
}

int ParseTreeCanvas::getMaxDepth(std::shared_ptr<ParseTreeNode> node, int currentDepth) {
    if (!node) return currentDepth;

    int maxChildDepth = currentDepth;
    for (const auto& child : node->getChildren()) {
        int childDepth = getMaxDepth(child, currentDepth + 1);
        maxChildDepth = qMax(maxChildDepth, childDepth);
    }

    return maxChildDepth;
}

int ParseTreeCanvas::countLeaves(std::shared_ptr<ParseTreeNode> node) {
    if (!node) return 0;

    if (node->getChildren().isEmpty()) {
        return 1;
    }

    int count = 0;
    for (const auto& child : node->getChildren()) {
        count += countLeaves(child);
    }

    return count;
}

// ParseTreeWidget Implementation

ParseTreeWidget::ParseTreeWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    canvas = new ParseTreeCanvas();

    scrollArea = new QScrollArea();
    scrollArea->setWidget(canvas);
    scrollArea->setWidgetResizable(false);  // Important: don't resize widget
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { "
        "   border: none; "
        "   background-color: #1e1e1e; "
        "}"
        "QScrollBar:horizontal {"
        "   background-color: #2b2b2b;"
        "   height: 12px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "   background-color: #555;"
        "   border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "   background-color: #666;"
        "}"
        "QScrollBar:vertical {"
        "   background-color: #2b2b2b;"
        "   width: 12px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #555;"
        "   border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #666;"
        "}"
        );

    layout->addWidget(scrollArea);
    setLayout(layout);
}

void ParseTreeWidget::setParseTree(const ParseTree& tree) {
    canvas->setParseTree(tree);
}

void ParseTreeWidget::clear() {
    canvas->clear();
}
