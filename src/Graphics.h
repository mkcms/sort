/* -*- mode: c++; -*- */
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <QGraphicsScene>
#include <QGraphicsView>

#include <unordered_map>
#include <unordered_set>

#include "SortItem.h"

class GraphicsView : public QGraphicsView {
    Q_OBJECT

  public:
    GraphicsView(QWidget *parent = nullptr);

    void resetZoom();
    void fitItemsInView();

  public slots:
    void setAntialiasingEnabled(bool enabled);

  protected:
    void resizeEvent(QResizeEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

  private:
    float m_zoomFactor = 1.0;
};

class SceneChanges {
  public:
    SceneChanges(int numItemsInVector);

    bool empty() const;

    int numItemsInVector() const;
    void addAccess(QGraphicsItem *);
    void addAssignment(QGraphicsItem *, int value);

    std::unordered_set<QGraphicsItem *> drainAccesses();
    std::unordered_map<QGraphicsItem *, int> drainAssignments();

  private:
    int m_numItemsInVector;

    std::unordered_map<QGraphicsItem *, int> m_assignments;
    std::unordered_set<QGraphicsItem *> m_accesses;
};

class Scene : public QGraphicsScene {
    Q_OBJECT

  public:
    void reset(std::vector<SortItem> &vec);

  public slots:
    void applyChanges(SceneChanges &);

  private:
    void unmarkItem(QGraphicsRectItem *item);
    void markItem(QGraphicsRectItem *item);

    QVector<QGraphicsRectItem *> m_markedItems;
};

#endif
