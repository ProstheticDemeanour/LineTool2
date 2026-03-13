#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// tabs/LineParametersTab.h
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
class QRadioButton;
class QStackedWidget;

namespace LineTool {

class TowerGeometryWidget;

class LineParametersTab : public QWidget
{
    Q_OBJECT
public:
    explicit LineParametersTab(Database& db, QWidget* parent = nullptr);

    // Called by LoadabilityTab to import current geometry/circuit settings
    GeometryInput currentInput() const { return collectSingle(); }

    GeometryInput      collectSingle() const;
    DualCircuitInput   collectDual()   const;

private slots:
    void onCalculate();
    void onConductorChanged(int index);
    void onGeometryChanged();
    void onCircuitModeChanged();   // single ↔ dual toggle

private:
    void buildLeftPane (QWidget* parent);
    void buildRightPane(QWidget* parent);
    void populateConductorCombo();

    void displaySingle(const LineResults&       r);
    void displayDual  (const DualCircuitResults& r);
    void clearOutputs ();

    // ── DB ────────────────────────────────────────────────────────────────────
    Database& m_db;

    // ── Mode toggle ───────────────────────────────────────────────────────────
    QRadioButton* m_rbSingle = nullptr;
    QRadioButton* m_rbDual   = nullptr;

    // ── Circuit 1 phase positions (shared for both modes) ─────────────────────
    QDoubleSpinBox* m_x1=nullptr; QDoubleSpinBox* m_y1=nullptr;
    QDoubleSpinBox* m_x2=nullptr; QDoubleSpinBox* m_y2=nullptr;
    QDoubleSpinBox* m_x3=nullptr; QDoubleSpinBox* m_y3=nullptr;

    // ── Circuit 2 phase positions (shown only in dual mode) ───────────────────
    QGroupBox*      m_gbCircuit2 = nullptr;
    QDoubleSpinBox* m_x4=nullptr; QDoubleSpinBox* m_y4=nullptr;
    QDoubleSpinBox* m_x5=nullptr; QDoubleSpinBox* m_y5=nullptr;
    QDoubleSpinBox* m_x6=nullptr; QDoubleSpinBox* m_y6=nullptr;

    // ── OHEW ──────────────────────────────────────────────────────────────────
    QCheckBox*      m_ohew1En=nullptr;
    QDoubleSpinBox* m_xo1=nullptr; QDoubleSpinBox* m_yo1=nullptr;
    QCheckBox*      m_ohew2En=nullptr;
    QDoubleSpinBox* m_xo2=nullptr; QDoubleSpinBox* m_yo2=nullptr;

    // ── Conductor ─────────────────────────────────────────────────────────────
    QComboBox*      m_conductorCombo=nullptr;
    QDoubleSpinBox* m_dsManual=nullptr;
    QSpinBox*       m_bundleNo=nullptr;
    QDoubleSpinBox* m_bundleSpace=nullptr;

    // ── Circuit ───────────────────────────────────────────────────────────────
    QDoubleSpinBox* m_voltageKV=nullptr;
    QDoubleSpinBox* m_lengthKm=nullptr;
    QDoubleSpinBox* m_freq=nullptr;
    QDoubleSpinBox* m_currentA=nullptr;

    // ── Tower widget ──────────────────────────────────────────────────────────
    TowerGeometryWidget* m_towerWidget=nullptr;

    // ── Right pane outputs ────────────────────────────────────────────────────
    // Geometry
    QLabel* m_lD12=nullptr; QLabel* m_lD23=nullptr;
    QLabel* m_lD13=nullptr; QLabel* m_lGMD=nullptr;
    // Dual extras (hidden in single mode)
    QGroupBox* m_gbDualGeom=nullptr;
    QLabel* m_lDSb=nullptr;
    QLabel* m_lDa1a2=nullptr; QLabel* m_lDb1b2=nullptr; QLabel* m_lDc1c2=nullptr;
    QLabel* m_lGMRl=nullptr;
    // Per-unit-length
    QLabel* m_lInductance=nullptr; QLabel* m_lCapacitance=nullptr;
    QLabel* m_lReactance=nullptr;  QLabel* m_lSusceptance=nullptr;
    // Characteristic
    QLabel* m_lZc=nullptr; QLabel* m_lK=nullptr; QLabel* m_lVelFactor=nullptr;
    // Power
    QLabel* m_lSIL=nullptr;       QLabel* m_lSILtotal=nullptr;
    QLabel* m_lQWave=nullptr;
    QLabel* m_lAdmittance=nullptr; QLabel* m_lCharging=nullptr;
    QLabel* m_lLoadability=nullptr;
    QGroupBox* m_gbSILtotal=nullptr;
    // ABCD
    QLabel* m_lA=nullptr; QLabel* m_lB=nullptr;
    QLabel* m_lC=nullptr; QLabel* m_lD=nullptr;

    QPushButton* m_calcBtn=nullptr;

    QMap<int, ElectricalRecord> m_elecCache;
};

} // namespace LineTool
