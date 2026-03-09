// ─────────────────────────────────────────────────────────────────────────────
// core/Database.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "Database.h"
#include <QSqlRecord>
#include <QUuid>

namespace LineTool {

Database::Database()
    : m_connectionName(QUuid::createUuid().toString())
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
}

Database::~Database()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::open(const QString& path)
{
    m_db.setDatabaseName(path);
    return m_db.open();
}

bool Database::isOpen() const { return m_db.isOpen(); }

QString Database::lastError() const
{
    return m_db.lastError().text();
}

// ── allConductors ─────────────────────────────────────────────────────────────
QVector<ConductorRecord> Database::allConductors() const
{
    QVector<ConductorRecord> out;
    QSqlQuery q(m_db);
    q.exec("SELECT uid, type, codename, area_mm2, stranding, overall_diameter_mm "
           "FROM conductor ORDER BY area_mm2");
    while (q.next()) {
        ConductorRecord r;
        r.uid                  = q.value(0).toInt();
        r.type                 = q.value(1).toString();
        r.codename             = q.value(2).toString();
        r.area_mm2             = q.value(3).toDouble();
        r.stranding            = q.value(4).toString();
        r.overall_diameter_mm  = q.value(5).toDouble();
        out.append(r);
    }
    return out;
}

// ── allMechanical ─────────────────────────────────────────────────────────────
QVector<MechanicalRecord> Database::allMechanical() const
{
    QVector<MechanicalRecord> out;
    QSqlQuery q(m_db);
    q.exec("SELECT uid, mass, breaking_load, modulus, expansion FROM mechanical");
    while (q.next()) {
        MechanicalRecord r;
        r.uid           = q.value(0).toInt();
        r.mass          = q.value(1).toDouble();
        r.breaking_load = q.value(2).toDouble();
        r.modulus       = q.value(3).toDouble();
        r.expansion     = q.value(4).toDouble();
        out.append(r);
    }
    return out;
}

// ── allElectrical ─────────────────────────────────────────────────────────────
QVector<ElectricalRecord> Database::allElectrical() const
{
    QVector<ElectricalRecord> out;
    QSqlQuery q(m_db);
    q.exec("SELECT uid, dc_resistance_20C, dc_resistance_75C, "
           "ac_resistance_50Hz, reactance_50Hz FROM electrical");
    while (q.next()) {
        ElectricalRecord r;
        r.uid                = q.value(0).toInt();
        r.dc_resistance_20C  = q.value(1).toDouble();
        r.dc_resistance_75C  = q.value(2).toDouble();
        r.ac_resistance_50Hz = q.value(3).toDouble();
        r.reactance_50Hz     = q.value(4).toDouble();
        out.append(r);
    }
    return out;
}

// ── allCurrentRatings ─────────────────────────────────────────────────────────
QVector<CurrentRatingRecord> Database::allCurrentRatings() const
{
    QVector<CurrentRatingRecord> out;
    QSqlQuery q(m_db);
    q.exec("SELECT uid, environment, season, time_of_day, wind_speed, amps "
           "FROM current_rating");
    while (q.next()) {
        CurrentRatingRecord r;
        r.uid         = q.value(0).toInt();
        r.environment = q.value(1).toString();
        r.season      = q.value(2).toString();
        r.time_of_day = q.value(3).toString();
        r.wind_speed  = q.value(4).toString();
        r.amps        = q.value(5).toDouble();
        out.append(r);
    }
    return out;
}

// ── conductorByName ───────────────────────────────────────────────────────────
QVector<ConductorRecord> Database::conductorByName(const QString& codename) const
{
    QVector<ConductorRecord> out;
    QSqlQuery q(m_db);
    q.prepare("SELECT uid, type, codename, area_mm2, stranding, overall_diameter_mm "
              "FROM conductor WHERE codename = ?");
    q.addBindValue(codename);
    q.exec();
    while (q.next()) {
        ConductorRecord r;
        r.uid                 = q.value(0).toInt();
        r.type                = q.value(1).toString();
        r.codename            = q.value(2).toString();
        r.area_mm2            = q.value(3).toDouble();
        r.stranding           = q.value(4).toString();
        r.overall_diameter_mm = q.value(5).toDouble();
        out.append(r);
    }
    return out;
}

// ── electricalByUid ───────────────────────────────────────────────────────────
ElectricalRecord Database::electricalByUid(int uid) const
{
    ElectricalRecord r{};
    QSqlQuery q(m_db);
    q.prepare("SELECT uid, dc_resistance_20C, dc_resistance_75C, "
              "ac_resistance_50Hz, reactance_50Hz FROM electrical WHERE uid=?");
    q.addBindValue(uid);
    q.exec();
    if (q.next()) {
        r.uid                = q.value(0).toInt();
        r.dc_resistance_20C  = q.value(1).toDouble();
        r.dc_resistance_75C  = q.value(2).toDouble();
        r.ac_resistance_50Hz = q.value(3).toDouble();
        r.reactance_50Hz     = q.value(4).toDouble();
    }
    return r;
}

// ── mechanicalByUid ───────────────────────────────────────────────────────────
MechanicalRecord Database::mechanicalByUid(int uid) const
{
    MechanicalRecord r{};
    QSqlQuery q(m_db);
    q.prepare("SELECT uid, mass, breaking_load, modulus, expansion "
              "FROM mechanical WHERE uid=?");
    q.addBindValue(uid);
    q.exec();
    if (q.next()) {
        r.uid           = q.value(0).toInt();
        r.mass          = q.value(1).toDouble();
        r.breaking_load = q.value(2).toDouble();
        r.modulus       = q.value(3).toDouble();
        r.expansion     = q.value(4).toDouble();
    }
    return r;
}

// ── conductorNames ────────────────────────────────────────────────────────────
QStringList Database::conductorNames() const
{
    QStringList names;
    QSqlQuery q(m_db);
    q.exec("SELECT codename FROM conductor ORDER BY area_mm2");
    while (q.next())
        names << q.value(0).toString();
    return names;
}

// ── fetchTable ────────────────────────────────────────────────────────────────
Database::TableData Database::fetchTable(const QString& tableName) const
{
    TableData td;

    // Whitelist to prevent SQL injection
    static const QStringList allowed = {"conductor", "electrical", "mechanical", "current_rating"};
    if (!allowed.contains(tableName.toLower()))
        return td;

    QSqlQuery q(m_db);
    q.exec(QString("SELECT * FROM %1").arg(tableName));

    QSqlRecord rec = q.record();
    for (int i = 0; i < rec.count(); ++i)
        td.headers << rec.fieldName(i);

    while (q.next()) {
        QVariantList row;
        for (int i = 0; i < rec.count(); ++i)
            row << q.value(i);
        td.rows << row;
    }
    return td;
}

} // namespace LineTool
