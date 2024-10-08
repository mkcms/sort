/* -*- mode: c++; -*- */
#ifndef SORTABLE_H
#define SORTABLE_H

#include <QGraphicsItem>
#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

enum class ArrayOrder {
    Ascending,
    Descending,
    Random,
    MostlySorted,
    PartiallySorted
};

inline constexpr int ArrayOrderCount = 5;

inline QString arrayOrderName(ArrayOrder order) {
    constexpr const char *ArrayOrderNames[] = {
        // clang-format off
        "Ascending",
        "Descending",
        "Random",
        "MostlySorted",
        "PartiallySorted",
        // clang-format on
    };
    return ArrayOrderNames[int(order)];
}

class SortItem;

struct SortItemCallbacks {
    virtual ~SortItemCallbacks() = default;

    virtual void onComparison(const SortItem &, const SortItem &) {}
    virtual void onAccess(const SortItem &) {}
    virtual void onAssignment(const SortItem &, int, int,
                              const SortItem * /*from*/ = nullptr) {}
};

class SortItem {
  public:
    SortItem() = default;
    SortItem(int);

    SortItem(const SortItem &);

    SortItem &operator=(const SortItem &);
    void swap(SortItem &);

    int value() const;
    operator int() const;

    QGraphicsItem *graphicsItem();
    const QGraphicsItem *graphicsItem() const;
    QGraphicsItem *mutableGraphicsItem() const;
    void setGraphicsItem(QGraphicsItem *item);

    std::strong_ordering operator<=>(const SortItem &rhs) const;
    bool operator==(const SortItem &rhs) const;

    static void setCallbacksForCurrentThread(SortItemCallbacks *);

  private:
    static thread_local SortItemCallbacks *callbacks;

    int m_value = 0;
    QGraphicsItem *m_graphicsItem = nullptr;
};

namespace std {
void swap(SortItem &, SortItem &);
}

std::vector<SortItem> generateVector(int numItems, ArrayOrder order);

#endif
