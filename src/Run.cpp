#include "Run.h"
#include "SortItem.h"
#include <qscopeguard.h>

static constexpr int FPS = 25;

class Interrupt : public std::exception {};

class Run::Callbacks : public SortItemCallbacks {
  public:
    Callbacks(Run &run) : m_run(run) {}

    void onComparison(const SortItem &lhs, const SortItem &rhs) override {
        QMutexLocker<QMutex> lock(&m_run.shared.mutex);
        commonCallback(lock);

        if (lhs.graphicsItem()) {
            m_run.shared.sceneChanges.addAccess(lhs.mutableGraphicsItem());
        }
        if (rhs.graphicsItem()) {
            m_run.shared.sceneChanges.addAccess(rhs.mutableGraphicsItem());
        }
        m_run.shared.stats.comparisons++;
        m_run.shared.stats.accesses += 2;
    }

    void onAccess(const SortItem &item) override {
        QMutexLocker<QMutex> lock(&m_run.shared.mutex);
        commonCallback(lock);

        if (item.graphicsItem()) {
            m_run.shared.sceneChanges.addAccess(item.mutableGraphicsItem());
        }

        m_run.shared.stats.accesses++;
    }

    void onAssignment(const SortItem &item, int /*oldValue*/, int newValue,
                      const SortItem * /*from*/ = nullptr) override {
        QMutexLocker<QMutex> lock(&m_run.shared.mutex);
        commonCallback(lock);

        if (item.graphicsItem()) {
            m_run.shared.sceneChanges.addAssignment(item.mutableGraphicsItem(),
                                                    newValue);
            m_run.shared.stats.accesses++;
        }
    }

  private:
    void commonCallback(QMutexLocker<QMutex> &lock) {
        // Assumes lock is held
        while (m_run.shared.pauseRequested && !m_run.shared.stopRequested) {
            lock.unlock();
            QScopeGuard guard([&] { lock.relock(); });
            QThread::usleep(10000);
        }
        if (m_run.shared.stopRequested) {
            throw Interrupt();
        }

        lock.unlock();
        QScopeGuard guard([&] { lock.relock(); });
        QThread::usleep(m_run.shared.delay.count());
    }

    Run &m_run;
};

Run::WorkerThread::WorkerThread(const std::function<void()> &func,
                                QObject *parent)
    : QThread(parent), m_func(func) {}

void Run::WorkerThread::run() { m_func(); }

Run::Run(std::vector<SortItem> &vec, std::chrono::microseconds delay,
         QObject *parent)
    : QObject(parent), m_vector(vec), m_state(State::NotStarted), m_timer(-1),
      m_callbacks(nullptr), m_thread(nullptr),
      shared{{}, false, false, delay, {static_cast<int>(vec.size())}, Stats{}} {
}

Run::~Run() {
    if (m_state != State::Finished && m_state != State::NotStarted) {
        stop();
    }

    if (m_thread) {
        m_thread->wait();
    }

    delete m_callbacks;
}

Run::State Run::state() const { return m_state; }

bool Run::start(const Algorithm &algorithm) {
    if (m_state != State::NotStarted) {
        return false;
    }

    m_state = State::Running;
    emit stateChanged(m_state);

    m_timer = startTimer(1000 / FPS);

    m_callbacks = new Callbacks(*this);

    auto func = algorithm.function;

    m_thread = new WorkerThread(
        [this, func] {
            SortItem::setCallbacksForCurrentThread(m_callbacks);
            try {
                func(m_vector);
            } catch (Interrupt &) {
            }
        },
        this);

    connect(m_thread, SIGNAL(finished()), this, SLOT(stop()));

    m_thread->start();

    return true;
}

bool Run::stop() {
    if (m_state != State::Running && m_state != State::Paused) {
        return false;
    }

    {
        QMutexLocker<QMutex> lock(&shared.mutex);
        shared.stopRequested = true;
    }

    m_thread->wait();
    // Drain twice: the first one will contain real changes, if any,
    // the second drain will be empty but will force the scene to
    // unmark all items.
    maybeDrainChanges();
    maybeDrainChanges(/*force=*/true);

    m_state = State::Finished;
    emit stateChanged(m_state);

    if (m_timer != -1) {
        // Running state.
        killTimer(m_timer);
        m_timer = -1;
    }

    return true;
}

bool Run::pause() {
    if (m_state != State::Running) {
        return false;
    }

    {
        QMutexLocker<QMutex> lock(&shared.mutex);
        shared.pauseRequested = true;
    }

    m_state = State::Paused;
    emit stateChanged(m_state);

    killTimer(m_timer);
    m_timer = -1;

    return true;
}

bool Run::resume() {
    if (m_state != State::Paused) {
        return false;
    }

    {
        QMutexLocker<QMutex> lock(&shared.mutex);
        shared.pauseRequested = false;
    }

    m_state = State::Running;
    emit stateChanged(m_state);

    m_timer = startTimer(1000 / FPS);

    return true;
}

void Run::setDelay(std::chrono::microseconds delay) {
    QMutexLocker<QMutex> lock(&shared.mutex);
    shared.delay = delay;
}

void Run::timerEvent(QTimerEvent *) { maybeDrainChanges(); }

void Run::maybeDrainChanges(bool force) {
    std::optional<SceneChanges> changes;
    Stats stats;

    {
        QMutexLocker<QMutex> lock(&shared.mutex);
        stats = shared.stats;
        if (!shared.sceneChanges.empty() || force) {
            auto size = shared.sceneChanges.numItemsInVector();
            changes = std::move(shared.sceneChanges);
            shared.sceneChanges = SceneChanges(size);
        }
    }

    if (changes) {
        emit sceneChangesReady(*changes);
    }
    emit statsReady(stats);
}
