/* -*- mode: c++; -*- */
#ifndef RUN_H
#define RUN_H

#include "Algorithms.h"
#include "Graphics.h"
#include "SortItem.h"

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QThread>

class Run : public QObject {
    Q_OBJECT
  public:
    struct Stats {
        int accesses = 0;
        int comparisons = 0;
    };

    Run(std::vector<SortItem> &vec, std::chrono::microseconds delay,
        QObject *parent = nullptr);

    ~Run();

    enum class State {
        NotStarted,
        Running,
        Paused,
        Finished,
    };
    Q_ENUM(State)

    State state() const;

  public slots:
    bool start(const Algorithm &);
    bool stop();
    bool pause();
    bool resume();
    void setDelay(std::chrono::microseconds);

  signals:
    void stateChanged(Run::State);
    void sceneChangesReady(SceneChanges &);
    void statsReady(Run::Stats);

  protected:
    void timerEvent(QTimerEvent *) override;

  private:
    void maybeDrainChanges(bool force = false);

    class WorkerThread;
    class Callbacks;
    friend class Callbacks;

    std::vector<SortItem> &m_vector;
    State m_state;
    int m_timer;
    Callbacks *m_callbacks;
    WorkerThread *m_thread;

    struct Shared {
        QMutex mutex;
        bool stopRequested;
        bool pauseRequested;
        std::chrono::microseconds delay;
        SceneChanges sceneChanges;
        Stats stats;
    } shared;
};

class Run::WorkerThread : public QThread {
    Q_OBJECT

  public:
    WorkerThread(const std::function<void()> &func, QObject *parent = nullptr);

  private:
    void run() override;

  private:
    std::function<void()> m_func;
};

#endif
