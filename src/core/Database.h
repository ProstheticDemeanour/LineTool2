#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// core/Database.h
//
// Thin Qt-Sql wrapper around conductors.db
// Schema matches the YAML specification exactly.
// ─────────────────────────────────────────────────────────────────────────────
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariantList>
#include <QVector>
#include <QStringList>
#include <stdexcept>

namespace LineTool {

// ─────────────────────────────────────────────────────────────────────────────
// Structs matching DB tables
// ─────────────────────────────────────────────────────────────────────────────
struct ConductorRecord {
    int     uid;
    QString type;
    QString codename;
    double  area_mm2;
    QString stranding;
    double  overall_diameter_mm;
};

struct MechanicalRecord {
    int    uid;
    double mass;            // kg/km
    double breaking_load;   // N
    double modulus;         // GPa
    double expansion;       // 1/K
};

struct ElectricalRecord {
    int    uid;
    double dc_resistance_20C;   // Ω/km
    double dc_resistance_75C;   // Ω/km
    double ac_resistance_50Hz;  // Ω/km
    double reactance_50Hz;      // Ω/km
};

struct CurrentRatingRecord {
    int     uid;
    QString environment;
    QString season;
    QString time_of_day;
    QString wind_speed;
    double  amps;
};

// ─────────────────────────────────────────────────────────────────────────────
// Database
// ─────────────────────────────────────────────────────────────────────────────
class Database
{
public:
    Database();
    ~Database();

    bool open(const QString& path);
    bool isOpen() const;
    QString lastError() const;

    // ── Conductor queries ──────────────────────────────────────────────────────
    QVector<ConductorRecord>    allConductors() const;
    QVector<MechanicalRecord>   allMechanical() const;
    QVector<ElectricalRecord>   allElectrical() const;
    QVector<CurrentRatingRecord> allCurrentRatings() const;

    QVector<ConductorRecord>    conductorByName(const QString& codename) const;
    ElectricalRecord            electricalByUid(int uid) const;
    MechanicalRecord            mechanicalByUid(int uid) const;

    QStringList conductorNames() const;

    // ── Generic table fetch for ConductorDatabaseTab ─────────────────────────
    // Returns {headers, rows} for any of "conductor", "electrical", "mechanical"
    struct TableData {
        QStringList             headers;
        QVector<QVariantList>   rows;
    };
    TableData fetchTable(const QString& tableName) const;

private:
    QSqlDatabase m_db;
    QString      m_connectionName;
};

} // namespace LineTool
