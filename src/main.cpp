#include "Algorithms.h"
#include "MainWindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption runTests("tests", "Test all algorithms and exit");
    parser.addOption(runTests);

    parser.process(app);

    if (parser.isSet(runTests)) {
        TestAlgorithms();
        return EXIT_SUCCESS;
    }

    MainWindow w;
    w.show();

    return app.exec();
}
