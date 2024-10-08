#include "SortItem.h"
#include <QApplication>
#include <QScopeGuard>
#include <compare>
#include <qapplication.h>
#include <random>
#include <thread>

static SortItemCallbacks DefaultCallbacks;
thread_local SortItemCallbacks *SortItem::callbacks = &DefaultCallbacks;

SortItem::SortItem(int value) : m_value(value), m_graphicsItem(nullptr) {}

SortItem::SortItem(const SortItem &other)
    : m_value(other.m_value), m_graphicsItem(other.m_graphicsItem) {}

SortItem &SortItem::operator=(const SortItem &rhs) {
    if (&rhs != this) {
        callbacks->onAssignment(*this, m_value, rhs.m_value, &rhs);
        m_value = rhs.m_value;
    }
    return *this;
}

void SortItem::swap(SortItem &rhs) {
    if (&rhs != this) {
        callbacks->onAssignment(*this, m_value, rhs.m_value, &rhs);
        callbacks->onAssignment(rhs, rhs.m_value, m_value, this);
        std::swap(m_value, rhs.m_value);
    }
}

int SortItem::value() const {
    callbacks->onAccess(*this);
    return m_value;
}

SortItem::operator int() const { return value(); }

QGraphicsItem *SortItem::graphicsItem() { return m_graphicsItem; }

const QGraphicsItem *SortItem::graphicsItem() const { return m_graphicsItem; }
QGraphicsItem *SortItem::mutableGraphicsItem() const { return m_graphicsItem; }

void SortItem::setGraphicsItem(QGraphicsItem *item) { m_graphicsItem = item; }

std::strong_ordering SortItem::operator<=>(const SortItem &rhs) const {
    callbacks->onComparison(*this, rhs);
    return std::strong_order(m_value, rhs.m_value);
}

bool SortItem::operator==(const SortItem &rhs) const {
    callbacks->onComparison(*this, rhs);
    return m_value == rhs.m_value;
}

void SortItem::setCallbacksForCurrentThread(SortItemCallbacks *cbs) {
    callbacks = cbs;
}

namespace std {
void swap(SortItem &lhs, SortItem &rhs) { lhs.swap(rhs); }
} // namespace std

std::vector<SortItem> generateVector(int numItems, ArrayOrder order) {
    std::vector<SortItem> ret;

    switch (order) {
    case ArrayOrder::Ascending:
        for (int i = 0; i < numItems; i++) {
            ret.emplace_back(i);
        }
        break;
    case ArrayOrder::Descending:
        for (int i = numItems - 1; i >= 0; i--) {
            ret.emplace_back(i);
        }
        break;
    case ArrayOrder::Random: {
        for (int i = 0; i < numItems; i++) {
            ret.emplace_back(i);
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(ret.begin(), ret.end(), g);

        break;
    }
    case ArrayOrder::MostlySorted: {
        for (int i = 0; i < numItems; i++) {
            ret.emplace_back(i);
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<int> uid(1, 9);

        auto it = ret.begin();

        while (it < ret.end()) {
            int size = uid(g);
            auto end = it + size;
            while (end > ret.end()) {
                end--;
            }
            std::shuffle(it, end, g);
            it = end;
        }

        break;
    }
    case ArrayOrder::PartiallySorted: {
        std::vector<int> values;
        for (int i = 0; i < numItems; i++) {
            values.emplace_back(i);
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<int> uid(0, numItems / 3);

        std::shuffle(values.begin(), values.end(), g);

        auto it = values.begin() + uid(g);

        while (it < values.end()) {
            int size = uid(g);
            auto end = it + size;
            while (end > values.end()) {
                end--;
            }
            std::sort(it, end);
            end += uid(g);
            it = end;
        }

        for (auto v : values) {
            ret.emplace_back(v);
        }

        break;
    }
    }

    return ret;
}
