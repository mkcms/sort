/* -*- mode: c++; -*- */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

#include "Run.h"
#include "SortItem.h"
#include "ui_MainWindow.h"

struct Algorithm;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setup();

  public slots:
    void onNumItemsChanged(int);
    void onOrderSelected(QListWidgetItem *);
    void onAlgorithmSelected(QListWidgetItem *);
    void onDelayChanged(int);
    void onRunClicked();
    void onPauseClicked();
    void onResumeClicked();
    void onResetClicked();

    void onRunStateChanged(Run::State);
    void onStats(Run::Stats);

  private:
    Ui_MainWindow *m_ui;

    struct {
        int numItems = 0;
        ArrayOrder order = ArrayOrder::Ascending;
        const Algorithm *algorithm = nullptr;
        std::chrono::microseconds delay = std::chrono::microseconds(0);
        bool needsRegenerate = false;
    } m_params;

    std::vector<SortItem> m_vector;

    Run *m_run = nullptr;
};

#endif
