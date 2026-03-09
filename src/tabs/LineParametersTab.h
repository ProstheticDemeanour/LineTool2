#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// tabs/LineParametersTab.h
//
// Tab 1: Line Geometry + Conductor selection (left pane)
//         + Computed parameters output (right pane)
//         + Real-time tower geometry drawing (bottom-left)
// ─────────────────────────────────────────────────────────────────────────────
#include <QWidget>
#include "core/Database.h"
#include "core/TransmissionLine.h"

class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QGroupBox;
class QPushButton;
class QFormLayout;
class QTableWidget;

namespace LineTool {

class TowerGeometryWidget;

class LineParametersTab : public QWidget
{
    Q_OBJECT
public:
    explicit LineParametersTab(Database& db, QWidget* parent = nullptr);

private slots:
    void onCalculate();
    void onConductorChanged(int index);
    void onGeometryChanged();      // live update of tower drawing

private:
    void buildLeftPane (QWidget* parent);
    void buildRightPane(QWidget* parent);
    void populateConductorCombo();
    GeometryInput collectInput() const;
    void displayResults(const LineResults& r);

    // ── DB ────────────────────────────────────────────────────────────────────
    Database& m_db;

    // ── Left pane — geometry inputs ───────────────────────────────────────────
    // Phase A
    QDoubleSpinBox* m_x1 = nullptr; QDoubleSpinBox* m_y1 = nullptr;
    // Phase B
    QDoubleSpinBox* m_x2 = nullptr; QDoubleSpinBox* m_y2 = nullptr;
    // Phase C
    QDoubleSpinBox* m_x3 = nullptr; QDoubleSpinBox* m_y3 = nullptr;

    // OHEW 1
    QCheckBox*      m_ohew1En = nullptr;
    QDoubleSpinBox* m_xo1 = nullptr; QDoubleSpinBox* m_yo1 = nullptr;
    // OHEW 2
    QCheckBox*      m_ohew2En = nullptr;
    QDoubleSpinBox* m_xo2 = nullptr; QDoubleSpinBox* m_yo2 = nullptr;

    // Conductor
    QComboBox*      m_conductorCombo = nullptr;
    QDoubleSpinBox* m_bundleSpace    = nullptr;
    QSpinBox*       m_bundleNo       = nullptr;
    QDoubleSpinBox* m_dsManual       = nullptr;   // GMR — auto-filled from DB

    // Circuit
    QDoubleSpinBox* m_voltageKV  = nullptr;
    QDoubleSpinBox* m_lengthKm   = nullptr;
    QDoubleSpinBox* m_freq       = nullptr;
    QDoubleSpinBox* m_currentA   = nullptr;

    // Tower drawing
    TowerGeometryWidget* m_towerWidget = nullptr;

    // ── Right pane — outputs ──────────────────────────────────────────────────
    // Geometry
    QLabel* m_lD12       = nullptr;
    QLabel* m_lD23       = nullptr;
    QLabel* m_lD13       = nullptr;
    QLabel* m_lGMD       = nullptr;

    // Per-unit-length
    QLabel* m_lInductance    = nullptr;
    QLabel* m_lCapacitance   = nullptr;
    QLabel* m_lReactance     = nullptr;
    QLabel* m_lSusceptance   = nullptr;

    // Characteristic
    QLabel* m_lZc            = nullptr;
    QLabel* m_lK             = nullptr;
    QLabel* m_lVelFactor     = nullptr;

    // Power
    QLabel* m_lSIL           = nullptr;
    QLabel* m_lQWave         = nullptr;
    QLabel* m_lAdmittance    = nullptr;
    QLabel* m_lCharging      = nullptr;
    QLabel* m_lLoadability   = nullptr;

    // ABCD
    QLabel* m_lA = nullptr, *m_lB = nullptr;
    QLabel* m_lC = nullptr, *m_lD = nullptr;

    QPushButton* m_calcBtn = nullptr;

    // Cache of conductor electrical data keyed by conductor uid
    QMap<int, ElectricalRecord> m_elecCache;
    QMap<int, int>              m_comboUidMap;   // combo index → uid
};

} // namespace LineTool
