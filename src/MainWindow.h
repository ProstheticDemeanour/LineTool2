#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// MainWindow.h
//
// Top-level window. Owns the QTabWidget and the Database instance.
//
// ADDING A NEW TAB:
//   1.  Create src/tabs/MyNewTab.h + .cpp   (inherit QWidget)
//   2.  #include it here
//   3.  Add a  MyNewTab* m_myTab;  member
//   4.  In buildUi():  m_myTab = new MyNewTab(m_db);
//                      m_tabs->addTab(m_myTab, "Icon  Title");
//   That's it.
// ─────────────────────────────────────────────────────────────────────────────
#include <QMainWindow>
#include "core/Database.h"

#include "tabs/LineParametersTab.h"
#include "tabs/ConductorDatabaseTab.h"
#include "tabs/LoadabilityTab.h"

namespace LineTool {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void setupDatabase();
    void buildUi();
    void setupStatusBar();

    // ── Shared resources ──────────────────────────────────────────────────────
    Database m_db;

    // ── UI elements ───────────────────────────────────────────────────────────
    QTabWidget*           m_tabs        = nullptr;
    LineParametersTab*    m_lineTab     = nullptr;
    ConductorDatabaseTab* m_dbTab       = nullptr;
    LoadabilityTab*       m_loadTab     = nullptr;

    QLabel* m_dbStatus = nullptr;
};

} // namespace LineTool
