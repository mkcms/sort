/* -*- mode: c++; -*- */
#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "SortItem.h"
#include <QString>
#include <functional>

struct Algorithm {
    QString name;
    std::function<void(std::vector<SortItem> &)> function;
};

const QVector<Algorithm> &GetAlgorithms();

#endif
