#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// models/ConductorTableModel.h
//
// Generic read-only table model fed by Database::TableData.
// Used by the ConductorDatabaseTab QTableView.
// ─────────────────────────────────────────────────────────────────────────────
#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>
#include <QVariantList>

namespace LineTool {

class ConductorTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ConductorTableModel(QObject* parent = nullptr);

    // Feed new data into the model
    void setTableData(const QStringList& headers,
                      const QVector<QVariantList>& rows);

    // QAbstractTableModel interface
    int      rowCount   (const QModelIndex& parent = {}) const override;
    int      columnCount(const QModelIndex& parent = {}) const override;
    QVariant data       (const QModelIndex& index,
                         int role = Qt::DisplayRole)      const override;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole)      const override;

private:
    QStringList           m_headers;
    QVector<QVariantList> m_rows;
};

} // namespace LineTool
