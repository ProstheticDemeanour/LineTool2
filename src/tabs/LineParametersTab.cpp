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
#include <QRadioButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QFrame>
#include <QShortcut>
#include <QKeySequence>
#include <cmath>

namespace LineTool {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
static QLabel* makeValueLabel(const QString& init = "—")
{
    auto* l = new QLabel(init);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    l->setMinimumWidth(140);
    l->setFrameShape(QFrame::StyledPanel);
    l->setStyleSheet("QLabel{font-family:monospace;font-size:11px;"
                     "background:palette(window);padding:1px 4px;}");
    return l;
}

static QDoubleSpinBox* makeSpin(double mn, double mx, double val,
                                  double step=0.1, int dp=3)
{
    auto* s = new QDoubleSpinBox();
    s->setRange(mn, mx); s->setValue(val);
    s->setSingleStep(step); s->setDecimals(dp);
    return s;
}

static QWidget* makeXYRow(double xv, double yv,
                           QDoubleSpinBox*& xsb, QDoubleSpinBox*& ysb)
{
    auto* w  = new QWidget();
    auto* hl = new QHBoxLayout(w);
    hl->setContentsMargins(0,0,0,0);
    xsb = makeSpin(-50, 50, xv, 0.5, 2);
    ysb = makeSpin(  0, 80, yv, 0.5, 2);
    hl->addWidget(new QLabel("x:"));
    hl->addWidget(xsb);
    hl->addWidget(new QLabel("y:"));
    hl->addWidget(ysb);
    return w;
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

    auto* leftScroll = new QScrollArea();
    leftScroll->setWidgetResizable(true);
    leftScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftScroll->setMinimumWidth(290);
    leftScroll->setMaximumWidth(400);

    auto* leftWidget = new QWidget();
    buildLeftPane(leftWidget);
    leftScroll->setWidget(leftWidget);

    auto* rightWidget = new QWidget();
    buildRightPane(rightWidget);

    root->addWidget(leftScroll, 0);
    root->addWidget(rightWidget, 1);

    auto* sc = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(sc, &QShortcut::activated, this, &LineParametersTab::onCalculate);

    populateConductorCombo();
    onCircuitModeChanged();   // sets initial visibility
    onGeometryChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// buildLeftPane
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::buildLeftPane(QWidget* parent)
{
    auto* vl = new QVBoxLayout(parent);
    vl->setSpacing(6);

    // ── Mode toggle ───────────────────────────────────────────────────────────
    {
        auto* gb  = new QGroupBox("Circuit Mode");
        auto* hl  = new QHBoxLayout(gb);
        auto* grp = new QButtonGroup(this);
        m_rbSingle = new QRadioButton("Single Circuit");
        m_rbDual   = new QRadioButton("Dual Circuit");
        m_rbSingle->setChecked(true);
        grp->addButton(m_rbSingle);
        grp->addButton(m_rbDual);
        hl->addWidget(m_rbSingle);
        hl->addWidget(m_rbDual);
        vl->addWidget(gb);
        connect(m_rbSingle, &QRadioButton::toggled,
                this, &LineParametersTab::onCircuitModeChanged);
        connect(m_rbDual,   &QRadioButton::toggled,
                this, &LineParametersTab::onCircuitModeChanged);
    }

    // ── Circuit 1 phases ──────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Circuit 1 — Phase Positions (m)");
        auto* fl = new QFormLayout(gb);
        fl->addRow("Phase A:", makeXYRow(-4.0, 10.0, m_x1, m_y1));
        fl->addRow("Phase B:", makeXYRow( 0.0, 12.0, m_x2, m_y2));
        fl->addRow("Phase C:", makeXYRow( 4.0, 10.0, m_x3, m_y3));
        vl->addWidget(gb);
        for (auto* sb : {m_x1,m_y1,m_x2,m_y2,m_x3,m_y3})
            connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &LineParametersTab::onGeometryChanged);
    }

    // ── Circuit 2 phases (shown only in dual mode) ────────────────────────────
    {
        m_gbCircuit2 = new QGroupBox("Circuit 2 — Phase Positions (m)");
        auto* fl = new QFormLayout(m_gbCircuit2);
        // Default: mirror of circuit 1 on opposite side
        fl->addRow("Phase A2:", makeXYRow( 8.0, 10.0, m_x4, m_y4));
        fl->addRow("Phase B2:", makeXYRow(12.0, 12.0, m_x5, m_y5));
        fl->addRow("Phase C2:", makeXYRow( 8.0, 10.0, m_x6, m_y6));
        vl->addWidget(m_gbCircuit2);
        for (auto* sb : {m_x4,m_y4,m_x5,m_y5,m_x6,m_y6})
            connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &LineParametersTab::onGeometryChanged);
    }

    // ── OHEWs ─────────────────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Earth Wire Positions (m)");
        auto* fl = new QFormLayout(gb);

        auto makeOhewRow = [&](double xv, double yv, bool en,
                               QCheckBox*& chk,
                               QDoubleSpinBox*& xsb, QDoubleSpinBox*& ysb)
        {
            auto* w  = new QWidget();
            auto* hl = new QHBoxLayout(w);
            hl->setContentsMargins(0,0,0,0);
            chk = new QCheckBox(); chk->setChecked(en);
            xsb = makeSpin(-50,50,xv,0.5,2);
            ysb = makeSpin(  0,80,yv,0.5,2);
            hl->addWidget(chk);
            hl->addWidget(new QLabel("x:"));
            hl->addWidget(xsb);
            hl->addWidget(new QLabel("y:"));
            hl->addWidget(ysb);
            return w;
        };

        fl->addRow("OHEW 1:", makeOhewRow(-3,16,false,m_ohew1En,m_xo1,m_yo1));
        fl->addRow("OHEW 2:", makeOhewRow( 3,16,false,m_ohew2En,m_xo2,m_yo2));
        vl->addWidget(gb);

        for (auto* sb : {m_xo1,m_yo1,m_xo2,m_yo2})
            connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &LineParametersTab::onGeometryChanged);
        connect(m_ohew1En,&QCheckBox::toggled,this,&LineParametersTab::onGeometryChanged);
        connect(m_ohew2En,&QCheckBox::toggled,this,&LineParametersTab::onGeometryChanged);
    }

    // ── Conductor ─────────────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Conductor");
        auto* fl = new QFormLayout(gb);
        m_conductorCombo = new QComboBox();
        fl->addRow("Type:",        m_conductorCombo);
        m_dsManual = makeSpin(0.001,0.5,0.0117,0.001,4);
        m_dsManual->setToolTip("GMR of one sub-conductor (m)");
        fl->addRow("GMR (m):",     m_dsManual);
        m_bundleNo = new QSpinBox(); m_bundleNo->setRange(1,4); m_bundleNo->setValue(1);
        fl->addRow("Bundle n:",    m_bundleNo);
        m_bundleSpace = makeSpin(0.1,1.0,0.40,0.05,2);
        fl->addRow("Bundle sp (m):", m_bundleSpace);
        vl->addWidget(gb);
        connect(m_conductorCombo,QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,&LineParametersTab::onConductorChanged);
    }

    // ── Circuit ───────────────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Circuit");
        auto* fl = new QFormLayout(gb);
        m_voltageKV = makeSpin(1,1200,220.0,10,1);
        m_lengthKm  = makeSpin(1,5000,100.0,10,1);
        m_freq      = makeSpin(1, 100, 50.0, 1,1);
        m_currentA  = makeSpin(1,5000,500.0,50,1);
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
        "QPushButton{font-weight:bold;font-size:12px;"
        "background:#2255aa;color:white;border-radius:4px;}"
        "QPushButton:hover{background:#3366cc;}"
        "QPushButton:pressed{background:#1a4488;}");
    connect(m_calcBtn,&QPushButton::clicked,this,&LineParametersTab::onCalculate);
    vl->addWidget(m_calcBtn);

    // ── Tower drawing ─────────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Tower Cross-Section");
        auto* bl = new QVBoxLayout(gb);
        m_towerWidget = new TowerGeometryWidget();
        m_towerWidget->setMinimumHeight(220);
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
        -> QGroupBox*
    {
        auto* gb = new QGroupBox(title);
        auto* fl = new QFormLayout(gb);
        fl->setLabelAlignment(Qt::AlignRight);
        for (auto& [lbl, ptr] : fields) {
            *ptr = makeValueLabel();
            fl->addRow(lbl, *ptr);
        }
        vl->addWidget(gb);
        return gb;
    };

    addGroup("Geometry",{
        {"D₁₂ (m):", &m_lD12},
        {"D₂₃ (m):", &m_lD23},
        {"D₁₃ (m):", &m_lD13},
        {"GMD (m):",  &m_lGMD},
    });

    // Dual-only geometry group
    m_gbDualGeom = new QGroupBox("Dual-Circuit Geometry");
    {
        auto* fl = new QFormLayout(m_gbDualGeom);
        fl->setLabelAlignment(Qt::AlignRight);
        auto add = [&](const QString& lbl, QLabel*& ptr){
            ptr = makeValueLabel(); fl->addRow(lbl, ptr);
        };
        add("DS bundle (m):",  m_lDSb);
        add("Da1a2 (m):",      m_lDa1a2);
        add("Db1b2 (m):",      m_lDb1b2);
        add("Dc1c2 (m):",      m_lDc1c2);
        add("GMRl (m):",       m_lGMRl);
    }
    vl->addWidget(m_gbDualGeom);

    addGroup("Per-Unit-Length",{
        {"Inductance (mH/km):",  &m_lInductance},
        {"Capacitance (µF/km):", &m_lCapacitance},
        {"Reactance (Ω/km):",   &m_lReactance},
        {"Susceptance (µS/km):", &m_lSusceptance},
    });

    addGroup("Characteristic",{
        {"Zc (Ω):",      &m_lZc},
        {"β (rad/km):",  &m_lK},
        {"Vel. factor:", &m_lVelFactor},
    });

    // Power group — includes SIL total row (hidden in single mode)
    {
        auto* gb = new QGroupBox("Power System");
        auto* fl = new QFormLayout(gb);
        fl->setLabelAlignment(Qt::AlignRight);
        auto add = [&](const QString& lbl, QLabel*& ptr){
            ptr = makeValueLabel(); fl->addRow(lbl, ptr);
        };
        add("SIL per circuit (MVA):", m_lSIL);
        add("λ/4 length (km):",        m_lQWave);
        add("Shunt admittance (mS):",  m_lAdmittance);
        add("Charging (MVAR):",        m_lCharging);
        add("Loadability (MW):",       m_lLoadability);

        // Dual-only total SIL row — wrap in its own mini groupbox for visibility toggle
        m_gbSILtotal = new QGroupBox("Dual Circuit Total");
        auto* fl2 = new QFormLayout(m_gbSILtotal);
        m_lSILtotal = makeValueLabel();
        fl2->addRow("SIL total (MVA):", m_lSILtotal);
        fl->addRow(m_gbSILtotal);

        vl->addWidget(gb);
    }

    addGroup("ABCD Matrix  |mag|∠°",{
        {"A:", &m_lA}, {"B:", &m_lB},
        {"C:", &m_lC}, {"D:", &m_lD},
    });

    vl->addStretch();
    scroll->setWidget(inner);
    auto* ol = new QVBoxLayout(parent);
    ol->setContentsMargins(0,0,0,0);
    ol->addWidget(scroll);
}

// ─────────────────────────────────────────────────────────────────────────────
// populateConductorCombo
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::populateConductorCombo()
{
    m_conductorCombo->blockSignals(true);
    m_conductorCombo->clear();
    m_elecCache.clear();
    m_conductorCombo->addItem("— manual entry —", -1);
    if (m_db.isOpen()) {
        for (const auto& c : m_db.allConductors()) {
            QString label = QString("%1  %2  %3 mm²")
                                .arg(c.type,-6).arg(c.codename,-8)
                                .arg(c.area_mm2,0,'f',0);
            m_conductorCombo->addItem(label, c.uid);
            m_elecCache[c.uid] = m_db.electricalByUid(c.uid);
        }
    }
    m_conductorCombo->blockSignals(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// onConductorChanged
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onConductorChanged(int index)
{
    if (index <= 0) return;
    int uid = m_conductorCombo->itemData(index).toInt();
    if (uid > 0 && m_elecCache.contains(uid)) {
        (void)m_elecCache[uid];  // r_ac picked up in collectInput
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onCircuitModeChanged — show/hide circuit 2 inputs and dual output rows
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onCircuitModeChanged()
{
    const bool dual = m_rbDual->isChecked();
    m_gbCircuit2->setVisible(dual);
    m_gbDualGeom->setVisible(dual);
    m_gbSILtotal->setVisible(dual);

    // Update SIL label text
    m_lSIL->parentWidget();   // no-op — label text set in display functions

    clearOutputs();
    onGeometryChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onGeometryChanged — live tower redraw
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onGeometryChanged()
{
    if (m_rbDual->isChecked())
        m_towerWidget->setDualGeometry(collectDual());
    else
        m_towerWidget->setGeometry(collectSingle());
}

// ─────────────────────────────────────────────────────────────────────────────
// collectSingle / collectDual
// ─────────────────────────────────────────────────────────────────────────────
GeometryInput LineParametersTab::collectSingle() const
{
    GeometryInput g;
    g.x1=m_x1->value(); g.y1=m_y1->value();
    g.x2=m_x2->value(); g.y2=m_y2->value();
    g.x3=m_x3->value(); g.y3=m_y3->value();
    g.hasOhew1=m_ohew1En->isChecked();
    g.xo1=m_xo1->value(); g.yo1=m_yo1->value();
    g.hasOhew2=m_ohew2En->isChecked();
    g.xo2=m_xo2->value(); g.yo2=m_yo2->value();
    g.DS         = m_dsManual->value();
    g.bundleNo   = m_bundleNo->value();
    g.bundleSpace= m_bundleSpace->value();
    int idx = m_conductorCombo->currentIndex();
    int uid = (idx>0) ? m_conductorCombo->itemData(idx).toInt() : -1;
    if (uid>0 && m_elecCache.contains(uid))
        g.r_ac = m_elecCache[uid].ac_resistance_50Hz;
    g.voltageKV=m_voltageKV->value();
    g.lengthKm =m_lengthKm ->value();
    g.freq     =m_freq     ->value();
    g.currentA =m_currentA ->value();
    return g;
}

DualCircuitInput LineParametersTab::collectDual() const
{
    DualCircuitInput g;
    // Base (circuit 1 + common)
    static_cast<GeometryInput&>(g) = collectSingle();
    // Circuit 2
    g.x4=m_x4->value(); g.y4=m_y4->value();
    g.x5=m_x5->value(); g.y5=m_y5->value();
    g.x6=m_x6->value(); g.y6=m_y6->value();
    return g;
}

// ─────────────────────────────────────────────────────────────────────────────
// clearOutputs
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::clearOutputs()
{
    for (auto* l : {m_lD12,m_lD23,m_lD13,m_lGMD,
                    m_lDSb,m_lDa1a2,m_lDb1b2,m_lDc1c2,m_lGMRl,
                    m_lInductance,m_lCapacitance,m_lReactance,m_lSusceptance,
                    m_lZc,m_lK,m_lVelFactor,
                    m_lSIL,m_lSILtotal,m_lQWave,m_lAdmittance,
                    m_lCharging,m_lLoadability,
                    m_lA,m_lB,m_lC,m_lD})
        if (l) l->setText("—");
}

// ─────────────────────────────────────────────────────────────────────────────
// onCalculate
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::onCalculate()
{
    try {
        if (m_rbDual->isChecked())
            displayDual(TransmissionLine::computeDual(collectDual()));
        else
            displaySingle(TransmissionLine::compute(collectSingle()));
    } catch (const std::exception& ex) {
        m_lD12->setText(QString("Error: %1").arg(ex.what()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// displaySingle
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::displaySingle(const LineResults& r)
{
    GeometryInput g = collectSingle();
    auto dist = [](double xa,double ya,double xb,double yb){
        return std::hypot(xb-xa,yb-ya);
    };
    double D12=dist(g.x1,g.y1,g.x2,g.y2);
    double D23=dist(g.x2,g.y2,g.x3,g.y3);
    double D13=dist(g.x1,g.y1,g.x3,g.y3);
    double GMD=std::cbrt(D12*D23*D13);

    auto f=[](double v,int dp=4){return QString::number(v,'f',dp);};
    auto fp=[](double mag,double ang){
        return QString("%1 ∠ %2°").arg(mag,0,'f',5).arg(ang,0,'f',2);
    };

    m_lD12->setText(f(D12,3)+" m");
    m_lD23->setText(f(D23,3)+" m");
    m_lD13->setText(f(D13,3)+" m");
    m_lGMD->setText(f(GMD,3)+" m");

    m_lInductance ->setText(f(r.inductance_mH_km,   4)+" mH/km");
    m_lCapacitance->setText(f(r.capacitance_uF_km,  5)+" µF/km");
    m_lReactance  ->setText(f(r.reactance_ohm_km,   4)+" Ω/km");
    m_lSusceptance->setText(f(r.susceptance_S_km*1e6,4)+" µS/km");

    m_lZc      ->setText(f(r.Zc_ohm,   2)+" Ω");
    m_lK       ->setText(f(r.k_rad_km, 6)+" rad/km");
    m_lVelFactor->setText(f(r.vel_factor,4));

    m_lSIL      ->setText(f(r.SIL_MVA,     1)+" MVA");
    m_lQWave    ->setText(f(r.quarter_wave_km,1)+" km");
    m_lAdmittance->setText(f(r.admittance_S*1e3,4)+" mS");
    m_lCharging ->setText(f(r.charging_MVAR,2)+" MVAR");
    m_lLoadability->setText(f(r.loadability_MW,1)+" MW");

    m_lA->setText(fp(r.A_mag,r.A_ang));
    m_lB->setText(fp(r.B_mag,r.B_ang)+" Ω");
    m_lC->setText(fp(r.C_mag*1e3,r.C_ang)+" mS");
    m_lD->setText(fp(r.D_mag,r.D_ang));
}

// ─────────────────────────────────────────────────────────────────────────────
// displayDual
// ─────────────────────────────────────────────────────────────────────────────
void LineParametersTab::displayDual(const DualCircuitResults& r)
{
    DualCircuitInput g = collectDual();
    auto dist=[](double xa,double ya,double xb,double yb){
        return std::hypot(xb-xa,yb-ya);
    };
    // Use circuit 1 internal distances for D12/D23/D13 display
    double D12=dist(g.x1,g.y1,g.x2,g.y2);
    double D23=dist(g.x2,g.y2,g.x3,g.y3);
    double D13=dist(g.x1,g.y1,g.x3,g.y3);

    auto f=[](double v,int dp=4){return QString::number(v,'f',dp);};
    auto fp=[](double mag,double ang){
        return QString("%1 ∠ %2°").arg(mag,0,'f',5).arg(ang,0,'f',2);
    };

    m_lD12->setText(f(D12,3)+" m (Cct 1)");
    m_lD23->setText(f(D23,3)+" m (Cct 1)");
    m_lD13->setText(f(D13,3)+" m (Cct 1)");
    m_lGMD->setText(f(r.GMD_m,4)+" m (equiv.)");

    m_lDSb  ->setText(f(r.DSb_m,   4)+" m");
    m_lDa1a2->setText(f(r.Da1a2_m, 3)+" m");
    m_lDb1b2->setText(f(r.Db1b2_m, 3)+" m");
    m_lDc1c2->setText(f(r.Dc1c2_m, 3)+" m");
    m_lGMRl ->setText(f(r.GMRl_m,  4)+" m");

    m_lInductance ->setText(f(r.inductance_mH_km,   4)+" mH/km");
    m_lCapacitance->setText(f(r.capacitance_uF_km,  5)+" µF/km");
    m_lReactance  ->setText(f(r.reactance_ohm_km,   4)+" Ω/km");
    m_lSusceptance->setText(f(r.susceptance_S_km*1e6,4)+" µS/km");

    m_lZc      ->setText(f(r.Zc_ohm,   2)+" Ω");
    m_lK       ->setText(f(r.k_rad_km, 6)+" rad/km");
    m_lVelFactor->setText(f(r.vel_factor,4));

    m_lSIL      ->setText(f(r.SIL_MVA,     1)+" MVA");
    m_lSILtotal ->setText(f(r.SIL_total_MVA,1)+" MVA");
    m_lQWave    ->setText(f(r.quarter_wave_km,1)+" km");
    m_lAdmittance->setText(f(r.susceptance_S_km * g.lengthKm * 1e3, 4)+" mS");
    m_lCharging ->setText(f(r.charging_MVAR,2)+" MVAR/cct");
    m_lLoadability->setText(f(r.loadability_MW,1)+" MW/cct");

    m_lA->setText(fp(r.A_mag,r.A_ang));
    m_lB->setText(fp(r.B_mag,r.B_ang)+" Ω");
    m_lC->setText(fp(r.C_mag*1e3,r.C_ang)+" mS");
    m_lD->setText(fp(r.D_mag,r.D_ang));
}

} // namespace LineTool
