// ─────────────────────────────────────────────────────────────────────────────
// main.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LineTool");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("LineTool");

    // Use Fusion style for consistent cross-platform look
    app.setStyle(QStyleFactory::create("Fusion"));

    LineTool::MainWindow w;
    w.show();

    return app.exec();
}
