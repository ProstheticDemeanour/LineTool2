// ─────────────────────────────────────────────────────────────────────────────
// models/ConductorTableModel.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "ConductorTableModel.h"

namespace LineTool {

ConductorTableModel::ConductorTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

void ConductorTableModel::setTableData(const QStringList& headers,
                                        const QVector<QVariantList>& rows)
{
    beginResetModel();
    m_headers = headers;
    m_rows    = rows;
    endResetModel();
}

int ConductorTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_rows.size();
}

int ConductorTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_headers.size();
}

QVariant ConductorTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const auto& row = m_rows.at(index.row());
        if (index.column() < row.size())
            return row.at(index.column());
    }
    if (role == Qt::TextAlignmentRole) {
        // Right-align numeric columns
        const auto& row = m_rows.at(index.row());
        if (index.column() < row.size()) {
            const QVariant& v = row.at(index.column());
            if (v.typeId() == QMetaType::Double || v.typeId() == QMetaType::Int)
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return {};
}

QVariant ConductorTableModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        if (section < m_headers.size())
            return m_headers.at(section);
    } else {
        return section + 1;
    }
    return {};
}

} // namespace LineTool
