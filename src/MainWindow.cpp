// ─────────────────────────────────────────────────────────────────────────────
// MainWindow.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "MainWindow.h"
#include "tabs/LineParametersTab.h"
#include "tabs/ConductorDatabaseTab.h"

#include <QTabWidget>
#include <QStatusBar>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>

namespace LineTool {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("LineTool  v2.0");
    setMinimumSize(1100, 700);
    resize(1350, 820);

    setupDatabase();
    buildUi();
    setupStatusBar();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupDatabase
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupDatabase()
{
    // 1. Next to executable
    QString dbPath = QDir::currentPath() + "/conductors.db";
    if (!m_db.open(dbPath)) {
        // 2. User app-data directory
        QString fallback = QStandardPaths::writableLocation(
                               QStandardPaths::AppDataLocation) + "/conductors.db";
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        m_db.open(fallback);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// buildUi
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildUi()
{
    // ── Menu bar ──────────────────────────────────────────────────────────────
    auto* fileMenu = menuBar()->addMenu("&File");

    auto* openDbAction = fileMenu->addAction("&Open Database…");
    openDbAction->setShortcut(QKeySequence::Open);
    connect(openDbAction, &QAction::triggered, this, [this] {
        QString path = QFileDialog::getOpenFileName(
            this, "Open Conductors Database", QDir::currentPath(),
            "SQLite Database (*.db *.sqlite);;All files (*)");
        if (!path.isEmpty()) {
            m_db.open(path);
            m_dbStatus->setText(m_db.isOpen()
                ? "  DB: " + path + "  ✓"
                : "  DB: failed to open  ✗");
            m_dbStatus->setStyleSheet(m_db.isOpen()
                ? "color: green;" : "color: red;");
        }
    });

    fileMenu->addSeparator();
    auto* quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this] {
        QMessageBox::about(this, "LineTool",
            "<b>LineTool v2.0</b><br/><br/>"
            "Overhead transmission line parameter calculator.<br/><br/>"
            "<b>Tabs:</b><br/>"
            "&nbsp;&nbsp;<b>⚡ Line Parameters</b> — geometry, L, C, SIL, ABCD, loadability<br/>"
            "&nbsp;&nbsp;<b>📋 Conductor Database</b> — view conductors.db tables<br/><br/>"
            "All calculations use distributed-parameter (exact) model.<br/>"
            "Results are indicative — verify against manufacturer data.");
    });

    // ── Tab widget ────────────────────────────────────────────────────────────
    m_tabs = new QTabWidget();
    m_tabs->setDocumentMode(true);
    m_tabs->setTabPosition(QTabWidget::North);

    m_lineTab = new LineParametersTab(m_db);
    m_dbTab   = new ConductorDatabaseTab(m_db);
    m_loadTab = new LoadabilityTab(m_lineTab);

    m_tabs->addTab(m_lineTab,  "⚡  Line Parameters");
    m_tabs->addTab(m_loadTab,  "📈  Loadability");
    m_tabs->addTab(m_dbTab,    "📋  Conductor Database");

    // ─────────────────────────────────────────────────────────────────────────
    // TO ADD A NEW TAB: (see MainWindow.h for full instructions)
    //   #include "tabs/MyNewTab.h"
    //   m_myTab = new MyNewTab(m_db);
    //   m_tabs->addTab(m_myTab, "🗂  My Tab");
    // ─────────────────────────────────────────────────────────────────────────

    setCentralWidget(m_tabs);
}

// ─────────────────────────────────────────────────────────────────────────────
// setupStatusBar
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupStatusBar()
{
    m_dbStatus = new QLabel();
    if (m_db.isOpen())
        m_dbStatus->setText("  DB: conductors.db  ✓");
    else
        m_dbStatus->setText("  DB: not found — place conductors.db beside executable  ✗");

    m_dbStatus->setStyleSheet(m_db.isOpen() ? "color: green;" : "color: orange;");
    statusBar()->addPermanentWidget(m_dbStatus);
    statusBar()->addWidget(new QLabel("  F5 = Calculate  |  LineTool v2.0  "));
}

} // namespace LineTool
