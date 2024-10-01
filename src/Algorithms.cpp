#include "Algorithms.h"
#include "SortItem.h"
#include <algorithm>

#ifdef HAVE_BOOST
#include <boost/sort/sort.hpp>
#endif

template <typename It, typename MergeFn>
static void mergeSortImpl(It begin, It end, MergeFn mergeFn);
template <typename It> static void merge(const It, const It, const It);

template <typename It> static void quickSortImpl(It begin, It end);

namespace Wiki {
void Sort(std::vector<SortItem> &vec);
}

void QuickSort(std::vector<SortItem> &vec) {
    quickSortImpl(vec.begin(), vec.end());
}

void MergeSort(std::vector<SortItem> &vec) {
    mergeSortImpl(
        vec.begin(), vec.end(),
        [](auto first, auto middle, auto last) { merge(first, middle, last); });
}

void MergeSortStdInplaceMerge(std::vector<SortItem> &vec) {
    mergeSortImpl(vec.begin(), vec.end(),
                  [](auto first, auto middle, auto last) {
                      std::inplace_merge(first, middle, last);
                  });
}

void MergeSortStdMerge(std::vector<SortItem> &vec) {
    mergeSortImpl(vec.begin(), vec.end(),
                  [](auto first, auto middle, auto last) {
                      using T = std::decay_t<decltype(*first)>;
                      std::vector<T> temp;
                      temp.resize(last - first);
                      std::merge(first, middle, middle, last, temp.begin());
                      for (auto &v : temp) {
                          *first++ = v;
                      }
                  });
}

void BottomUpMergeSort(std::vector<SortItem> &vec) {
    for (unsigned int i = 1; i < vec.size(); i *= 2) {
        for (auto j = vec.begin(); j < vec.end();) {
            auto first = j;
            auto middle = std::min(j + i, vec.end());
            auto last = std::min(middle + i, vec.end());
            j = last;
            merge(first, middle, last);
        }
    }
}

void StdSort(std::vector<SortItem> &vec) { std::sort(vec.begin(), vec.end()); }

void StdStableSort(std::vector<SortItem> &vec) {
    std::stable_sort(vec.begin(), vec.end());
}

void StdSortHeap(std::vector<SortItem> &vec) {
    std::make_heap(vec.begin(), vec.end());
    std::sort_heap(vec.begin(), vec.end());
}

#ifdef HAVE_BOOST
void BoostPdqSort(std::vector<SortItem> &vec) {
    boost::sort::pdqsort(vec.begin(), vec.end());
}

void BoostSampleSort(std::vector<SortItem> &vec) {
    boost::sort::sample_sort(vec.begin(), vec.end());
}

void BoostSpinSort(std::vector<SortItem> &vec) {
    boost::sort::spinsort(vec.begin(), vec.end());
}

void BoostFlatStableSort(std::vector<SortItem> &vec) {
    boost::sort::flat_stable_sort(vec.begin(), vec.end());
}
#endif

void ShellSort(std::vector<SortItem> &vec) {
    unsigned int gaps[] = {
        929, 505, 209, 109, 41, 19, 5, 1,
    };

    for (auto gap : gaps) {
        for (unsigned int i = gap; i < vec.size(); i++) {
            auto temp = vec[i];
            unsigned int j;
            for (j = i; (j >= gap) && vec[j - gap] > temp; j -= gap) {
                vec[j] = vec[j - gap];
            }
            vec[j] = temp;
        }
    }
}

void InsertionSort(std::vector<SortItem> &vec) {
    for (unsigned i = 1; i < vec.size(); i++) {
        int j = i;
        while (j >= 1 && vec[j] < vec[j - 1]) {
            std::swap(vec[j], vec[j - 1]);
            j--;
        }
    }
}

void SelectionSort(std::vector<SortItem> &vec) {
    for (unsigned i = 0; i < vec.size(); i++) {
        auto *min = &vec[i];
        for (unsigned j = i + 1; j < vec.size(); j++) {
            if (vec[j] < *min) {
                min = &vec[j];
            }
        }
        std::swap(*min, vec[i]);
    }
}

void BubbleSort(std::vector<SortItem> &vec) {
    bool swapped = false;
    do {
        swapped = false;
        for (unsigned j = 1; j < vec.size(); j++) {
            if (vec[j] < vec[j - 1]) {
                swapped = true;
                std::swap(vec[j], vec[j - 1]);
            }
        }
    } while (swapped);
}

void CocktailSort(std::vector<SortItem> &vec) {
    bool swapped = false;
    do {
        swapped = false;
        for (unsigned j = 1; j < vec.size(); j++) {
            if (vec[j] < vec[j - 1]) {
                swapped = true;
                std::swap(vec[j], vec[j - 1]);
            }
        }
        if (!swapped) {
            break;
        }
        for (unsigned j = vec.size() - 1; j > 0; j--) {
            if (vec[j] < vec[j - 1]) {
                swapped = true;
                std::swap(vec[j], vec[j - 1]);
            }
        }
    } while (swapped);
}

template <typename It> static auto &choosePivot(It begin, It end) {
    auto min = begin, mid = begin + (end - begin) / 2, max = end - 1;

    if (min > mid) {
        std::swap(min, mid);
    }
    if (mid > max) {
        std::swap(mid, max);
    }
    if (min > mid) {
        std::swap(min, mid);
    }

    return *mid;
}

template <typename It> static void quickSortImpl(It begin, It end) {
    auto size = end - begin;
    if (size <= 1) {
        return;
    }

    auto *pivot = &choosePivot(begin, end);
    std::swap(*begin, *pivot);
    pivot = &*begin;

    auto endOfFirstPartition = std::partition(
        begin + 1, end, [&](const auto &elt) { return elt < *pivot; });

    auto beginOfSecondPartition =
        std::partition(endOfFirstPartition, end,
                       [&](const auto &elt) { return elt == *pivot; });

    quickSortImpl(begin, endOfFirstPartition);
    quickSortImpl(beginOfSecondPartition, end);
}

template <typename It>
static void merge(const It first, const It middle, const It last) {
    auto begFirst = first;
    auto endFirst = middle;
    auto begSecond = middle;
    auto endSecond = last;

    using T = std::decay_t<decltype(*first)>;

    std::vector<T> temp;

    while (begFirst != endFirst && begSecond != endSecond) {
        if (*begFirst <= *begSecond) {
            temp.emplace_back(*begFirst++);
        } else {
            temp.emplace_back(*begSecond++);
        }
    }
    while (begFirst != endFirst) {
        temp.emplace_back(*begFirst++);
    }
    while (begSecond != endSecond) {
        temp.emplace_back(*begSecond++);
    }

    auto it = first;
    for (auto &v : temp) {
        *it++ = v;
    }
}

template <typename It, typename MergeFn>
void mergeSortImpl(It begin, It end, MergeFn mergeFn) {
    auto size = end - begin;
    if (size <= 1) {
        return;
    }

    auto half = begin + size / 2;
    mergeSortImpl(begin, half, mergeFn);
    mergeSortImpl(half, end, mergeFn);

    mergeFn(begin, half, end);
}

const QVector<Algorithm> &GetAlgorithms() {
    static const QVector<Algorithm> algorithms = {
        {.name = "QuickSort", .function = QuickSort},
        {.name = "MergeSort", .function = MergeSort},
        {.name = "MergeSort (std::inplace_merge)",
         .function = MergeSortStdInplaceMerge},
        {.name = "MergeSort (std::merge)", .function = MergeSortStdMerge},
        {.name = "Bottom-Up MergeSort", .function = BottomUpMergeSort},
        {.name = "WikiSort", .function = Wiki::Sort},
        {.name = "std::sort", .function = StdSort},
        {.name = "std::stable_sort", .function = StdStableSort},
        {.name = "std::sort_heap", .function = StdSortHeap},
#ifdef HAVE_BOOST
        {.name = "boost::sort::pdqsort", .function = BoostPdqSort},
        {.name = "boost::sort::sample_sort", .function = BoostSampleSort},
        {.name = "boost::sort::spinsort", .function = BoostSpinSort},
        {.name = "boost::sort::flat_stable_sort",
         .function = BoostFlatStableSort},
#endif
        {.name = "ShellSort", .function = ShellSort},
        {.name = "InsertionSort", .function = InsertionSort},
        {.name = "SelectionSort", .function = SelectionSort},
        {.name = "BubbleSort", .function = BubbleSort},
        {.name = "CocktailSort", .function = CocktailSort},
    };

    return algorithms;
}
