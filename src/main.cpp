#include <QApplication>
#include <QMessageBox>
#include "TimeTracker.h"
#include "data/PostgresRepository.h"
#include <memory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 1. Establish data layer repository context instance
    auto repo = std::make_shared<PostgresRepository>();
    if (!repo->initializeConnection()) {
        QMessageBox::warning(nullptr, "Network Connection Warning", 
            "Could not establish persistent central PostgreSQL database link. Running in disconnected fallback state.");
    }

    // 2. Inject repository capability directly inside the presentation shell
    TimeTracker tracker(repo);
    tracker.show();

    return app.exec();
}
