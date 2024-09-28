#include "Graphics.h"
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPen>
#include <qgraphicsitem.h>

static constexpr int ITEM_WIDTH = 100;
static constexpr int ITEM_HEIGHT_MULT = 100;
static constexpr int ITEM_BORDER_WIDTH = 5;

static const auto ItemBrush = QBrush(Qt::white);
static const auto MarkedItemBrush = QBrush(Qt::red);
static const auto ItemPen = QPen(QBrush(Qt::black), ITEM_BORDER_WIDTH);

static const auto Background = QBrush(Qt::darkGray);

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void GraphicsView::fitItemsInView() {
    fitInView(scene()->itemsBoundingRect(), Qt::IgnoreAspectRatio);
}

void GraphicsView::resizeEvent(QResizeEvent *) { fitItemsInView(); }

SceneChanges::SceneChanges(int numItemsInVector)
    : m_numItemsInVector(numItemsInVector) {}

bool SceneChanges::empty() const {
    return m_accesses.empty() && m_assignments.empty();
}

int SceneChanges::numItemsInVector() const { return m_numItemsInVector; }

void SceneChanges::addAccess(QGraphicsItem *item) { m_accesses.insert(item); }

void SceneChanges::addAssignment(QGraphicsItem *item, int value) {
    m_assignments[item] = value;
}

std::unordered_set<QGraphicsItem *> SceneChanges::drainAccesses() {
    auto ret = std::move(m_accesses);
    m_accesses.clear();
    return ret;
}

std::unordered_map<QGraphicsItem *, int> SceneChanges::drainAssignments() {
    auto ret = std::move(m_assignments);
    m_assignments.clear();
    return ret;
}

void Scene::reset(std::vector<SortItem> &vector) {
    clear();
    m_markedItems.clear();

    setBackgroundBrush(Background);

    int numItems = vector.size();

    for (int i = 0; i < numItems; i++) {
        auto &sortable = vector[i];
        int value = sortable.value();

        QRectF rect;
        rect.setX(0);
        rect.setY(0);
        rect.setWidth(ITEM_WIDTH);
        rect.setHeight(value * ITEM_HEIGHT_MULT + ITEM_HEIGHT_MULT);

        auto x = i * ITEM_WIDTH;
        auto y = numItems * ITEM_HEIGHT_MULT - rect.height();

        auto *item = addRect(rect);
        item->setPos(x, y);

        item->setBrush(ItemBrush);
        item->setPen(ItemPen);
        sortable.setGraphicsItem(item);
    }
}

void Scene::applyChanges(SceneChanges &changes) {
    auto numItems = changes.numItemsInVector();

    auto assignments = changes.drainAssignments();
    auto accesses = changes.drainAccesses();

    for (auto *item : m_markedItems) {
        unmarkItem(item);
    }

    for (auto &[item, value] : assignments) {
        QGraphicsRectItem *it = static_cast<QGraphicsRectItem *>(item);
        auto r = it->rect();
        r.setHeight((value * ITEM_HEIGHT_MULT) + ITEM_HEIGHT_MULT);

        auto pos = it->pos();
        pos.setY((numItems * ITEM_HEIGHT_MULT) - r.height());

        it->setPos(pos);
        it->setRect(r);

        markItem(it);
    }

    for (auto *item : accesses) {
        QGraphicsRectItem *it = static_cast<QGraphicsRectItem *>(item);
        markItem(it);
    }
}

void Scene::unmarkItem(QGraphicsRectItem *item) { item->setBrush(ItemBrush); }

void Scene::markItem(QGraphicsRectItem *item) {
    item->setBrush(MarkedItemBrush);
    m_markedItems.push_back(item);
}
