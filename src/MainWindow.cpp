#include "MainWindow.h"

#include <QGraphicsScene>
#include <QStringListModel>
#include <algorithm>
#include <qnamespace.h>

#include "Algorithms.h"
#include "Graphics.h"
#include "SortItem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui_MainWindow) {
    m_ui->setupUi(this);

    const auto &algorithms = GetAlgorithms();
    for (const auto &algo : algorithms) {
        QListWidgetItem *item = new QListWidgetItem(algo.name);
        m_ui->listWidgetAlgorithms->addItem(item);
        item->setSelected(true);
    }
    for (int i = 0; i < ArrayOrderCount; i++) {
        const ArrayOrder order = static_cast<ArrayOrder>(i);
        const auto name = arrayOrderName(order);
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, static_cast<int>(i));
        m_ui->listWidgetItemOrder->addItem(item);
        item->setSelected(true);
    }

    connect(m_ui->spinBoxNumItems, SIGNAL(valueChanged(int)), this,
            SLOT(onNumItemsChanged(int)));
    connect(m_ui->listWidgetAlgorithms,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(onAlgorithmSelected(QListWidgetItem *)));
    connect(m_ui->listWidgetItemOrder,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(onOrderSelected(QListWidgetItem *)));
    connect(m_ui->pushButtonRun, SIGNAL(clicked()), this, SLOT(onRunClicked()));
    connect(m_ui->pushButtonPause, SIGNAL(clicked()), this,
            SLOT(onPauseClicked()));
    connect(m_ui->pushButtonResume, SIGNAL(clicked()), this,
            SLOT(onResumeClicked()));
    connect(m_ui->pushButtonReset, SIGNAL(clicked()), this,
            SLOT(onResetClicked()));
    connect(m_ui->listWidgetItemOrder,
            SIGNAL(itemDoubleClicked(QListWidgetItem *)), this,
            SLOT(onResetClicked()));
    connect(m_ui->dialDelay, SIGNAL(valueChanged(int)), this,
            SLOT(onDelayChanged(int)));

    m_ui->graphicsView->setScene(new Scene);

    onNumItemsChanged(m_ui->spinBoxNumItems->value());
    m_ui->listWidgetAlgorithms->setCurrentItem(
        m_ui->listWidgetAlgorithms->item(0));
    m_ui->listWidgetItemOrder->setCurrentItem(
        m_ui->listWidgetItemOrder->item(2));

    setup();
}

MainWindow::~MainWindow() { delete m_ui; }

void MainWindow::setup() {
    if (!m_params.algorithm) {
        // not yet selected.
        return;
    }

    if (m_run) {
        delete m_run;
        m_run = nullptr;
    }

    m_vector = generateVector(m_params.numItems, m_params.order);

    Scene *scene = qobject_cast<Scene *>(m_ui->graphicsView->scene());

    scene->reset(m_vector);

    m_ui->graphicsView->fitItemsInView();
    m_ui->graphicsView->resetZoom();

    m_run = new Run(m_vector, m_params.delay, this);
    connect(m_run, SIGNAL(stateChanged(Run::State)), this,
            SLOT(onRunStateChanged(Run::State)));
    connect(m_run, SIGNAL(sceneChangesReady(SceneChanges &)), scene,
            SLOT(applyChanges(SceneChanges &)));
    connect(m_run, SIGNAL(statsReady(Run::Stats)), this,
            SLOT(onStats(Run::Stats)));

    onRunStateChanged(Run::State::NotStarted);
    onStats(Run::Stats{});
    m_params.needsRegenerate = false;
}

void MainWindow::onNumItemsChanged(int numItems) {
    m_params.numItems = numItems;
    m_params.needsRegenerate = true;
}

void MainWindow::onOrderSelected(QListWidgetItem *item) {
    m_params.order = static_cast<ArrayOrder>(item->data(Qt::UserRole).toInt());
    m_params.needsRegenerate = true;
}

void MainWindow::onAlgorithmSelected(QListWidgetItem *item) {
    m_params.algorithm =
        &*std::ranges::find(GetAlgorithms(), item->text(), &Algorithm::name);
}

void MainWindow::onDelayChanged(int us) {
    m_params.delay = std::chrono::microseconds(us);
    m_ui->labelDelayValue->setText(QString::asprintf("%d us", us));
    m_run->setDelay(m_params.delay);
}

void MainWindow::onRunClicked() {
    if (m_run->state() == Run::State::Finished || m_params.needsRegenerate) {
        setup();
    }
    m_run->start(*m_params.algorithm);
}

void MainWindow::onPauseClicked() { m_run->pause(); }

void MainWindow::onResumeClicked() { m_run->resume(); }

void MainWindow::onResetClicked() { setup(); }

void MainWindow::onRunStateChanged(Run::State state) {
    switch (state) {
    case Run::State::NotStarted:
        m_ui->pushButtonRun->show();
        m_ui->pushButtonPause->hide();
        m_ui->pushButtonResume->hide();
        break;
    case Run::State::Running:
        m_ui->pushButtonRun->hide();
        m_ui->pushButtonPause->show();
        m_ui->pushButtonResume->hide();
        break;
    case Run::State::Paused:
        m_ui->pushButtonRun->hide();
        m_ui->pushButtonPause->hide();
        m_ui->pushButtonResume->show();
        break;
    case Run::State::Finished:
        m_ui->pushButtonRun->show();
        m_ui->pushButtonPause->hide();
        m_ui->pushButtonResume->hide();
        break;
    }
}

void MainWindow::onStats(Run::Stats stats) {
    m_ui->labelAccessesValue->setNum(stats.accesses);
    m_ui->labelComparisonsValue->setNum(stats.comparisons);
}
