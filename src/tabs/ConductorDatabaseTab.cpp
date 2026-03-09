// ─────────────────────────────────────────────────────────────────────────────
// tabs/ConductorDatabaseTab.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "ConductorDatabaseTab.h"
#include "models/ConductorTableModel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QFont>

namespace LineTool {

ConductorDatabaseTab::ConductorDatabaseTab(Database& db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    buildUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// buildUi
// ─────────────────────────────────────────────────────────────────────────────
void ConductorDatabaseTab::buildUi()
{
    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(8,8,8,8);
    vl->setSpacing(6);

    // ── Table selector ────────────────────────────────────────────────────────
    {
        auto* gb  = new QGroupBox("View Table");
        auto* hl  = new QHBoxLayout(gb);
        auto* grp = new QButtonGroup(this);

        struct { const char* label; const char* table; } tabs[] = {
            {"Conductor",       "conductor"},
            {"Electrical",      "electrical"},
            {"Mechanical",      "mechanical"},
            {"Current Ratings", "current_rating"},
        };

        for (auto& t : tabs) {
            auto* rb = new QRadioButton(t.label);
            rb->setProperty("table", QString(t.table));
            grp->addButton(rb);
            hl->addWidget(rb);
            if (QString(t.table) == "conductor")
                rb->setChecked(true);

            connect(rb, &QRadioButton::toggled, this, [this, rb](bool checked){
                if (checked)
                    onTableSelected(rb->property("table").toString());
            });
        }
        hl->addStretch();
        vl->addWidget(gb);
    }

    // ── Search / filter bar ───────────────────────────────────────────────────
    {
        auto* hl = new QHBoxLayout();
        hl->addWidget(new QLabel("🔍  Filter:"));
        m_searchEdit = new QLineEdit();
        m_searchEdit->setPlaceholderText("Type to filter any column…");
        m_searchEdit->setClearButtonEnabled(true);
        hl->addWidget(m_searchEdit);
        vl->addLayout(hl);
    }

    // ── Table view ────────────────────────────────────────────────────────────
    m_model      = new ConductorTableModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);   // search all columns

    m_tableView = new QTableView();
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->verticalHeader()->setDefaultSectionSize(22);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setShowGrid(true);

    QFont mono("Monospace");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(10);
    m_tableView->setFont(mono);

    vl->addWidget(m_tableView, 1);

    // ── Status bar ────────────────────────────────────────────────────────────
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: grey; font-size: 10px;");
    vl->addWidget(m_statusLabel);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &ConductorDatabaseTab::onFilterChanged);

    connect(m_proxyModel, &QSortFilterProxyModel::rowsInserted, this, [this](){
        m_statusLabel->setText(
            QString("  %1 rows").arg(m_proxyModel->rowCount()));
    });
    connect(m_proxyModel, &QSortFilterProxyModel::rowsRemoved, this, [this](){
        m_statusLabel->setText(
            QString("  %1 rows  (filtered)").arg(m_proxyModel->rowCount()));
    });
    connect(m_proxyModel, &QSortFilterProxyModel::modelReset, this, [this](){
        m_statusLabel->setText(
            QString("  %1 rows").arg(m_proxyModel->rowCount()));
    });

    // Load initial table
    loadTable("conductor");
}

// ─────────────────────────────────────────────────────────────────────────────
// onTableSelected
// ─────────────────────────────────────────────────────────────────────────────
void ConductorDatabaseTab::onTableSelected(const QString& tableName)
{
    if (tableName == m_currentTable) return;
    m_searchEdit->clear();
    loadTable(tableName);
}

// ─────────────────────────────────────────────────────────────────────────────
// loadTable
// ─────────────────────────────────────────────────────────────────────────────
void ConductorDatabaseTab::loadTable(const QString& tableName)
{
    m_currentTable = tableName;

    if (!m_db.isOpen()) {
        m_statusLabel->setText("  ⚠  Database not connected");
        m_model->setTableData({}, {});
        return;
    }

    Database::TableData td = m_db.fetchTable(tableName);
    m_model->setTableData(td.headers, td.rows);
    m_tableView->resizeColumnsToContents();
    m_statusLabel->setText(QString("  Table: %1   |   %2 rows")
                               .arg(tableName).arg(td.rows.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// onFilterChanged
// ─────────────────────────────────────────────────────────────────────────────
void ConductorDatabaseTab::onFilterChanged(const QString& text)
{
    m_proxyModel->setFilterFixedString(text);
    m_statusLabel->setText(
        QString("  %1 / %2 rows  (filter: \"%3\")")
            .arg(m_proxyModel->rowCount())
            .arg(m_model->rowCount())
            .arg(text.isEmpty() ? "none" : text));
}

} // namespace LineTool
