// ─────────────────────────────────────────────────────────────────────────────
// tabs/LineParametersTab.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "LineParametersTab.h"
#include "widgets/TowerGeometryWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QKeySequence>
#include <QShortcut>
#include <cmath>

namespace LineTool {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
static QLabel* makeValueLabel(const QString& init = "—")
{
    auto* l = new QLabel(init);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    l->setMinimumWidth(130);
    l->setFrameShape(QFrame::StyledPanel);
    l->setStyleSheet("QLabel { font-family: monospace; font-size: 11px; "
                     "background: palette(window); padding: 1px 4px; }");
    return l;
}

static QDoubleSpinBox* makeSpin(double min, double max, double val,
                                  double step = 0.1, int decimals = 3)
{
    auto* s = new QDoubleSpinBox();
    s->setRange(min, max);
    s->setValue(val);
    s->setSingleStep(step);
    s->setDecimals(decimals);
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
LineParametersTab::LineParametersTab(Database& db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(6,6,6,6);
    root->setSpacing(8);

    // Left pane in a scroll area (geometry + drawing)
    auto* leftScroll = new QScrollArea();
    leftScroll->setWidgetResizable(true);
    leftScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftScroll->setMinimumWidth(280);
    leftScroll->setMaximumWidth(380);

    auto* leftWidget = new QWidget();
    buildLeftPane(leftWidget);
    leftScroll->setWidget(leftWidget);

    // Right pane — outputs
    auto* rightWidget = new QWidget();
    buildRightPane(rightWidget);

    root->addWidget(leftScroll, 0);
    root->addWidget(rightWidget, 1);

    // F5 shortcut
    auto* sc = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(sc, &QShortcut::activated, this, &LineParametersTab::onCalculate);

    // Populate & initial draw
    populateConductorCombo();
    onGeometryChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// buildLeftPane
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::buildLeftPane(QWidget* parent)
{
    auto* vl = new QVBoxLayout(parent);
    vl->setSpacing(6);

    // ── Phase conductor positions ─────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Phase Positions (m)");
        auto* fl = new QFormLayout(gb);
        fl->setLabelAlignment(Qt::AlignRight);

        auto makeXY = [&](double xv, double yv,
                          QDoubleSpinBox*& xsb, QDoubleSpinBox*& ysb)
        {
            auto* row = new QWidget();
            auto* hl  = new QHBoxLayout(row);
            hl->setContentsMargins(0,0,0,0);
            xsb = makeSpin(-30, 30, xv, 0.5, 2);
            ysb = makeSpin(  0, 50, yv, 0.5, 2);
            hl->addWidget(new QLabel("x:"));
            hl->addWidget(xsb);
            hl->addWidget(new QLabel("y:"));
            hl->addWidget(ysb);
            return row;
        };

        fl->addRow("Phase A:", makeXY(-4.0, 10.0, m_x1, m_y1));
        fl->addRow("Phase B:", makeXY( 0.0, 12.0, m_x2, m_y2));
        fl->addRow("Phase C:", makeXY( 4.0, 10.0, m_x3, m_y3));

        vl->addWidget(gb);

        auto geomConnect = [&](QDoubleSpinBox* sb){
            connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &LineParametersTab::onGeometryChanged);
        };
        geomConnect(m_x1); geomConnect(m_y1);
        geomConnect(m_x2); geomConnect(m_y2);
        geomConnect(m_x3); geomConnect(m_y3);
    }

    // ── OHEW positions ────────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Earth Wire Positions (m)");
        auto* fl = new QFormLayout(gb);

        auto makeOhewRow = [&](double xv, double yv, bool en,
                               QCheckBox*& chk,
                               QDoubleSpinBox*& xsb, QDoubleSpinBox*& ysb) -> QWidget*
        {
            auto* row = new QWidget();
            auto* hl  = new QHBoxLayout(row);
            hl->setContentsMargins(0,0,0,0);
            chk = new QCheckBox();
            chk->setChecked(en);
            xsb = makeSpin(-30, 30, xv, 0.5, 2);
            ysb = makeSpin(  0, 50, yv, 0.5, 2);
            hl->addWidget(chk);
            hl->addWidget(new QLabel("x:"));
            hl->addWidget(xsb);
            hl->addWidget(new QLabel("y:"));
            hl->addWidget(ysb);
            return row;
        };

        fl->addRow("OHEW 1:", makeOhewRow(-3,16,false, m_ohew1En, m_xo1, m_yo1));
        fl->addRow("OHEW 2:", makeOhewRow( 3,16,false, m_ohew2En, m_xo2, m_yo2));

        vl->addWidget(gb);

        auto ohewConnect = [&](QCheckBox* cb, QDoubleSpinBox* xs, QDoubleSpinBox* ys){
            connect(cb, &QCheckBox::toggled,        this, &LineParametersTab::onGeometryChanged);
            connect(xs, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &LineParametersTab::onGeometryChanged);
            connect(ys, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &LineParametersTab::onGeometryChanged);
        };
        ohewConnect(m_ohew1En, m_xo1, m_yo1);
        ohewConnect(m_ohew2En, m_xo2, m_yo2);
    }

    // ── Conductor selection ───────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Conductor");
        auto* fl = new QFormLayout(gb);

        m_conductorCombo = new QComboBox();
        m_conductorCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        fl->addRow("Type:",        m_conductorCombo);

        m_dsManual = makeSpin(0.001, 0.5, 0.0117, 0.001, 4);
        m_dsManual->setToolTip("Geometric Mean Radius (m) — auto-filled from DB");
        fl->addRow("GMR (m):",     m_dsManual);

        m_bundleNo = new QSpinBox();
        m_bundleNo->setRange(1,4);
        m_bundleNo->setValue(1);
        fl->addRow("Bundle n:",    m_bundleNo);

        m_bundleSpace = makeSpin(0.1, 1.0, 0.40, 0.05, 2);
        fl->addRow("Bundle sp (m):", m_bundleSpace);

        vl->addWidget(gb);

        connect(m_conductorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &LineParametersTab::onConductorChanged);
    }

    // ── Circuit parameters ────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Circuit");
        auto* fl = new QFormLayout(gb);

        m_voltageKV = makeSpin(1, 1200, 220.0, 10, 1);
        m_lengthKm  = makeSpin(1, 5000, 100.0, 10, 1);
        m_freq      = makeSpin(1,  100,  50.0,  1, 1);
        m_currentA  = makeSpin(1, 5000, 500.0, 50, 1);

        fl->addRow("Voltage (kV):",   m_voltageKV);
        fl->addRow("Length (km):",    m_lengthKm);
        fl->addRow("Frequency (Hz):", m_freq);
        fl->addRow("I thermal (A):",  m_currentA);

        vl->addWidget(gb);
    }

    // ── Calculate button ──────────────────────────────────────────────────────
    m_calcBtn = new QPushButton("⚡  Calculate  [F5]");
    m_calcBtn->setFixedHeight(34);
    m_calcBtn->setStyleSheet(
        "QPushButton { font-weight: bold; font-size: 12px; "
        "background: #2255aa; color: white; border-radius: 4px; }"
        "QPushButton:hover { background: #3366cc; }"
        "QPushButton:pressed { background: #1a4488; }");
    connect(m_calcBtn, &QPushButton::clicked, this, &LineParametersTab::onCalculate);
    vl->addWidget(m_calcBtn);

    // ── Tower geometry drawing ─────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Tower Cross-Section");
        auto* bl = new QVBoxLayout(gb);
        m_towerWidget = new TowerGeometryWidget();
        m_towerWidget->setMinimumHeight(200);
        bl->addWidget(m_towerWidget);
        vl->addWidget(gb);
    }

    vl->addStretch();
}

// ─────────────────────────────────────────────────────────────────────────────
// buildRightPane
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::buildRightPane(QWidget* parent)
{
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    auto* inner = new QWidget();
    auto* vl    = new QVBoxLayout(inner);
    vl->setSpacing(8);

    auto addGroup = [&](const QString& title,
                        std::initializer_list<std::pair<QString,QLabel**>> fields)
    {
        auto* gb = new QGroupBox(title);
        auto* fl = new QFormLayout(gb);
        fl->setLabelAlignment(Qt::AlignRight);
        for (auto& [lbl, ptr] : fields) {
            *ptr = makeValueLabel();
            fl->addRow(lbl, *ptr);
        }
        vl->addWidget(gb);
    };

    addGroup("Geometry",{
        {"D₁₂ (m):",  &m_lD12},
        {"D₂₃ (m):",  &m_lD23},
        {"D₁₃ (m):",  &m_lD13},
        {"GMD (m):",  &m_lGMD},
    });

    addGroup("Per-Unit-Length",{
        {"Inductance (mH/km):",    &m_lInductance},
        {"Capacitance (µF/km):",   &m_lCapacitance},
        {"Reactance  (Ω/km):",     &m_lReactance},
        {"Susceptance (S/km):",    &m_lSusceptance},
    });

    addGroup("Characteristic",{
        {"Zc (Ω):",              &m_lZc},
        {"β (rad/km):",          &m_lK},
        {"Vel. factor:",         &m_lVelFactor},
    });

    addGroup("Power System",{
        {"SIL (MVA):",           &m_lSIL},
        {"λ/4 length (km):",     &m_lQWave},
        {"Shunt admittance (S):", &m_lAdmittance},
        {"Charging (MVAR):",     &m_lCharging},
        {"Loadability (MW):",    &m_lLoadability},
    });

    addGroup("ABCD Matrix  |mag|∠°",{
        {"A :", &m_lA},
        {"B :", &m_lB},
        {"C :", &m_lC},
        {"D :", &m_lD},
    });

    vl->addStretch();
    scroll->setWidget(inner);

    auto* outerLayout = new QVBoxLayout(parent);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->addWidget(scroll);
}

// ─────────────────────────────────────────────────────────────────────────────
// populateConductorCombo
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::populateConductorCombo()
{
    m_conductorCombo->blockSignals(true);
    m_conductorCombo->clear();
    m_comboUidMap.clear();
    m_elecCache.clear();

    m_conductorCombo->addItem("— manual entry —", -1);

    if (m_db.isOpen()) {
        auto conductors = m_db.allConductors();
        for (int i = 0; i < conductors.size(); ++i) {
            const auto& c = conductors[i];
            QString label = QString("%1  %2  %3 mm²")
                                .arg(c.type, -6)
                                .arg(c.codename, -8)
                                .arg(c.area_mm2, 0, 'f', 0);
            m_conductorCombo->addItem(label, c.uid);
            m_comboUidMap[i+1] = c.uid;

            // Cache electrical record
            m_elecCache[c.uid] = m_db.electricalByUid(c.uid);
        }
    }

    m_conductorCombo->blockSignals(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// onConductorChanged — auto-fill DS from DB
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onConductorChanged(int index)
{
    if (index <= 0) return;   // manual entry
    int uid = m_conductorCombo->itemData(index).toInt();
    if (uid < 0) return;

    if (m_elecCache.contains(uid)) {
        // GMR/DS is conductor-specific; r_ac is picked up in collectInput().
        // Extend here to auto-fill m_dsManual when your DB carries a GMR column.
        (void)m_elecCache[uid];
    }

    // Update conductor DB info in status bar via parent window — optional hook point
}

// ─────────────────────────────────────────────────────────────────────────────
// onGeometryChanged — live tower redraw
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onGeometryChanged()
{
    GeometryInput g = collectInput();
    m_towerWidget->setGeometry(g);
}

// ─────────────────────────────────────────────────────────────────────────────
// collectInput
// ─────────────────────────────────────────────────────────────────────────────
GeometryInput LineParametersTab::collectInput() const
{
    GeometryInput g;
    g.x1 = m_x1->value(); g.y1 = m_y1->value();
    g.x2 = m_x2->value(); g.y2 = m_y2->value();
    g.x3 = m_x3->value(); g.y3 = m_y3->value();

    g.hasOhew1 = m_ohew1En->isChecked();
    g.xo1 = m_xo1->value(); g.yo1 = m_yo1->value();
    g.hasOhew2 = m_ohew2En->isChecked();
    g.xo2 = m_xo2->value(); g.yo2 = m_yo2->value();

    g.DS          = m_dsManual->value();
    g.bundleNo    = m_bundleNo->value();
    g.bundleSpace = m_bundleSpace->value();

    // AC resistance from DB if a conductor is selected
    int idx = m_conductorCombo->currentIndex();
    int uid = (idx > 0) ? m_conductorCombo->itemData(idx).toInt() : -1;
    if (uid > 0 && m_elecCache.contains(uid))
        g.r_ac = m_elecCache[uid].ac_resistance_50Hz;

    g.voltageKV = m_voltageKV->value();
    g.lengthKm  = m_lengthKm ->value();
    g.freq      = m_freq     ->value();
    g.currentA  = m_currentA ->value();

    return g;
}

// ─────────────────────────────────────────────────────────────────────────────
// onCalculate
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onCalculate()
{
    GeometryInput g = collectInput();

    try {
        LineResults r = TransmissionLine::compute(g);
        displayResults(r);
    } catch (const std::exception& ex) {
        // Show error in first label
        m_lD12->setText(QString("Error: %1").arg(ex.what()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// displayResults
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::displayResults(const LineResults& r)
{
    auto fmt = [](double v, int dp=4) {
        return QString::number(v, 'f', dp);
    };
    auto fmtPolar = [](double mag, double ang) {
        return QString("%1 ∠ %2°")
                   .arg(mag,  0, 'f', 5)
                   .arg(ang, 0, 'f', 2);
    };

    // Geometry — recompute distances from current input
    GeometryInput g = collectInput();
    auto dist = [](double x1,double y1,double x2,double y2){
        return std::hypot(x2-x1, y2-y1);
    };
    double D12 = dist(g.x1,g.y1,g.x2,g.y2);
    double D23 = dist(g.x2,g.y2,g.x3,g.y3);
    double D13 = dist(g.x1,g.y1,g.x3,g.y3);
    double GMD = std::cbrt(D12*D23*D13);

    m_lD12->setText(fmt(D12,3) + " m");
    m_lD23->setText(fmt(D23,3) + " m");
    m_lD13->setText(fmt(D13,3) + " m");
    m_lGMD->setText(fmt(GMD,3) + " m");

    m_lInductance ->setText(fmt(r.inductance_mH_km,   4) + " mH/km");
    m_lCapacitance->setText(fmt(r.capacitance_uF_km,  5) + " µF/km");
    m_lReactance  ->setText(fmt(r.reactance_ohm_km,   4) + " Ω/km");
    m_lSusceptance->setText(fmt(r.susceptance_S_km * 1e6, 4) + " µS/km");

    m_lZc       ->setText(fmt(r.Zc_ohm,      2) + " Ω");
    m_lK        ->setText(fmt(r.k_rad_km,     6) + " rad/km");
    m_lVelFactor->setText(fmt(r.vel_factor,   4));

    m_lSIL       ->setText(fmt(r.SIL_MVA,      1) + " MVA");
    m_lQWave     ->setText(fmt(r.quarter_wave_km, 1) + " km");
    m_lAdmittance->setText(fmt(r.admittance_S * 1e3, 4) + " mS");
    m_lCharging  ->setText(fmt(r.charging_MVAR, 2) + " MVAR");
    m_lLoadability->setText(fmt(r.loadability_MW, 1) + " MW");

    m_lA->setText(fmtPolar(r.A_mag, r.A_ang));
    m_lB->setText(fmtPolar(r.B_mag, r.B_ang) + " Ω");
    m_lC->setText(fmtPolar(r.C_mag * 1e3, r.C_ang) + " mS");
    m_lD->setText(fmtPolar(r.D_mag, r.D_ang));
}

} // namespace LineTool
