#include "Graphics.h"
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QPen>
#include <QWheelEvent>

static constexpr int ITEM_WIDTH = 100;
static constexpr int ITEM_HEIGHT_MULT = 100;
static constexpr int ITEM_BORDER_WIDTH = 5;

static const auto ItemBrush = QBrush(Qt::white);
static const auto MarkedItemBrush = QBrush(Qt::red);
static const auto ItemPen = QPen(QBrush(Qt::black), ITEM_BORDER_WIDTH);

static const auto Background = QBrush(Qt::darkGray);

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(ViewportUpdateMode::BoundingRectViewportUpdate);
}

void GraphicsView::resetZoom() {
    m_zoomFactor = 1.0;
    fitItemsInView();
}

void GraphicsView::fitItemsInView() {
    QRectF boundingRect = scene()->itemsBoundingRect();
    QRectF newRect = boundingRect;
    newRect.setWidth(boundingRect.width() / m_zoomFactor);
    fitInView(newRect, Qt::IgnoreAspectRatio);
    scene()->setSceneRect(boundingRect);
}

void GraphicsView::setAntialiasingEnabled(bool enabled) {
    setRenderHint(QPainter::Antialiasing, enabled);
}

void GraphicsView::resizeEvent(QResizeEvent *event) {
    fitItemsInView();
    QGraphicsView::resizeEvent(event);
}

void GraphicsView::wheelEvent(QWheelEvent *ev) {
    const double zoomIn = 1.03, zoomOut = 1 / zoomIn, maxZoomIn = 4.0;

    if (ev->angleDelta().y() == 0) {
        return;
    }

    double zoom, newScale;
    if (ev->angleDelta().y() > 0)
        zoom = zoomIn;
    else
        zoom = zoomOut;
    newScale = m_zoomFactor * zoom;

    if (newScale <= 1) {
        m_zoomFactor = 1;
        fitItemsInView();
        setDragMode(QGraphicsView::NoDrag);
    } else if (newScale >= maxZoomIn) {
        m_zoomFactor = maxZoomIn;
    } else {
        QPointF pos = mapToScene(ev->position().toPoint()), pos2, d;

        scale(zoom, 1.0);
        pos2 = mapToScene(pos.toPoint());

        d = pos - pos2;
        translate(d.x(), 0);

        m_zoomFactor = newScale;
        setDragMode(QGraphicsView::ScrollHandDrag);
    }
}

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
