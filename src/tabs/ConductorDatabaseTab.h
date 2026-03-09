#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// tabs/ConductorDatabaseTab.h
//
// Tab 2: Conductor database viewer
//   • RadioButton group selects which table to view
//     (conductor | electrical | mechanical | current_rating)
//   • QTableView + ConductorTableModel renders rows read from SQLite
//   • Filter/search bar above the table
// ─────────────────────────────────────────────────────────────────────────────
#include <QWidget>
#include "core/Database.h"

class QTableView;
class QRadioButton;
class QLineEdit;
class QLabel;
class QSortFilterProxyModel;

namespace LineTool {

class ConductorTableModel;

class ConductorDatabaseTab : public QWidget
{
    Q_OBJECT
public:
    explicit ConductorDatabaseTab(Database& db, QWidget* parent = nullptr);

private slots:
    void onTableSelected(const QString& tableName);
    void onFilterChanged(const QString& text);

private:
    void buildUi();
    void loadTable(const QString& tableName);

    Database&                m_db;
    ConductorTableModel*     m_model       = nullptr;
    QSortFilterProxyModel*   m_proxyModel  = nullptr;
    QTableView*              m_tableView   = nullptr;
    QLineEdit*               m_searchEdit  = nullptr;
    QLabel*                  m_statusLabel = nullptr;
    QString                  m_currentTable;
};

} // namespace LineTool
