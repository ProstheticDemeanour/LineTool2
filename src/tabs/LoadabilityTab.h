#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// tabs/LoadabilityTab.h
//
// Loadability curve tab.
//
// Left pane:
//   • Import toggle — pull L/C/Zc/V from LineParametersTab, or enter manually
//   • One collapsible "Line N" card per line (start with 1, add up to 8)
//   • Curve visibility checkboxes (Practical, Stability, St.Clair, SIL, Thermal)
//   • Calculate [F5] button
//
// Right pane:
//   • LoadabilityChartWidget (QtCharts)
//   • Summary table — one row per line showing SIL, Zc, λ/4, P_thermal
// ─────────────────────────────────────────────────────────────────────────────
#include <QWidget>
#include <QVector>
#include "core/LoadabilityCalc.h"
#include "core/Database.h"

class QDoubleSpinBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QGroupBox;
class QTableWidget;
class QScrollArea;

namespace LineTool {

class LoadabilityChartWidget;
class LineParametersTab;

// ── One line's input card ─────────────────────────────────────────────────────
struct LineCard {
    QGroupBox*      group    = nullptr;
    QLineEdit*      lblEdit  = nullptr;   // line name
    QPushButton*    removeBtn= nullptr;
    QCheckBox*      importCb = nullptr;   // import from LineParametersTab

    QDoubleSpinBox* L_sb     = nullptr;   // mH/km
    QDoubleSpinBox* C_sb     = nullptr;   // µF/km
    QDoubleSpinBox* r_sb     = nullptr;   // Ω/km
    QDoubleSpinBox* Vs_sb    = nullptr;   // kV
    QDoubleSpinBox* Vr_sb    = nullptr;   // kV
    QDoubleSpinBox* Vrate_sb = nullptr;   // kV
    QDoubleSpinBox* Ith_sb   = nullptr;   // A
    QDoubleSpinBox* delta_sb = nullptr;   // degrees
    QDoubleSpinBox* freq_sb  = nullptr;   // Hz
    QDoubleSpinBox* maxLen_sb= nullptr;   // km
};

class LoadabilityTab : public QWidget
{
    Q_OBJECT
public:
    // lineTab may be nullptr — import will be disabled if so
    explicit LoadabilityTab(LineParametersTab* lineTab,
                            QWidget* parent = nullptr);

    // Called by MainWindow whenever LineParametersTab recalculates
    void notifyLineParamsUpdated();

public slots:
    void onCalculate();

private slots:
    void onAddLine();
    void onRemoveLine(int index);
    void onImportToggled(int index, bool checked);

private:
    void buildLeftPane (QWidget* p);
    void buildRightPane(QWidget* p);

    LineCard* makeLineCard(int index);
    void      addCardToLayout(LineCard* card);
    void      updateCardIndices();
    LoadabilityInput collectCard(const LineCard& card) const;
    void      importFromLineTab(LineCard& card) const;

    // ── References ────────────────────────────────────────────────────────────
    LineParametersTab* m_lineTab = nullptr;

    // ── Left pane ─────────────────────────────────────────────────────────────
    QVBoxLayout*  m_cardsLayout  = nullptr;   // card container
    QScrollArea*  m_cardsScroll  = nullptr;
    QWidget*      m_cardsWidget  = nullptr;
    QPushButton*  m_addBtn       = nullptr;
    QPushButton*  m_calcBtn      = nullptr;

    // Curve visibility checkboxes
    QCheckBox* m_cbPractical  = nullptr;
    QCheckBox* m_cbStability  = nullptr;
    QCheckBox* m_cbStClair    = nullptr;
    QCheckBox* m_cbSIL        = nullptr;
    QCheckBox* m_cbThermal    = nullptr;

    // ── Right pane ────────────────────────────────────────────────────────────
    LoadabilityChartWidget* m_chart     = nullptr;
    QTableWidget*           m_table     = nullptr;

    // ── State ─────────────────────────────────────────────────────────────────
    QVector<LineCard*> m_cards;
    static constexpr int kMaxLines = 8;
};

} // namespace LineTool
