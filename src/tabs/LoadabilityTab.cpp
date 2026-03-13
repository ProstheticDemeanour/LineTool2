// ─────────────────────────────────────────────────────────────────────────────
// tabs/LoadabilityTab.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "LoadabilityTab.h"
#include "widgets/LoadabilityChartWidget.h"
#include "LineParametersTab.h"
#include "core/TransmissionLine.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QSplitter>
#include <QKeySequence>
#include <cmath>

namespace LineTool {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
static QDoubleSpinBox* dSpin(double mn, double mx, double val,
                               double step=1.0, int dp=4)
{
    auto* s = new QDoubleSpinBox();
    s->setRange(mn,mx); s->setValue(val);
    s->setSingleStep(step); s->setDecimals(dp);
    s->setMinimumWidth(90);
    return s;
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
LoadabilityTab::LoadabilityTab(LineParametersTab* lineTab, QWidget* parent)
    : QWidget(parent), m_lineTab(lineTab)
{
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(6,6,6,6);
    root->setSpacing(6);

    // Left pane
    auto* leftWidget = new QWidget();
    leftWidget->setMinimumWidth(270);
    leftWidget->setMaximumWidth(360);
    buildLeftPane(leftWidget);

    // Right pane — splitter: chart on top, summary table below
    auto* rightWidget = new QWidget();
    buildRightPane(rightWidget);

    root->addWidget(leftWidget, 0);
    root->addWidget(rightWidget, 1);

    // F5
    auto* sc = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(sc, &QShortcut::activated, this, &LoadabilityTab::onCalculate);

    // Seed one line card
    onAddLine();
}

// ─────────────────────────────────────────────────────────────────────────────
// buildLeftPane
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::buildLeftPane(QWidget* p)
{
    auto* vl = new QVBoxLayout(p);
    vl->setSpacing(6);
    vl->setContentsMargins(4,4,4,4);

    // ── Curve visibility ──────────────────────────────────────────────────────
    {
        auto* gb = new QGroupBox("Show Curves");
        auto* gl = new QVBoxLayout(gb);
        m_cbPractical = new QCheckBox("Practical loadability");  m_cbPractical->setChecked(true);
        m_cbStability = new QCheckBox("Theoretical stability");  m_cbStability->setChecked(true);
        m_cbStClair   = new QCheckBox("St. Clair curve");        m_cbStClair  ->setChecked(true);
        m_cbSIL       = new QCheckBox("SIL");                    m_cbSIL      ->setChecked(true);
        m_cbThermal   = new QCheckBox("Thermal limit");          m_cbThermal  ->setChecked(true);
        for (auto* cb : {m_cbPractical,m_cbStability,m_cbStClair,m_cbSIL,m_cbThermal})
            gl->addWidget(cb);
        vl->addWidget(gb);

        // Connect visibility toggles directly — no recalculate needed
        connect(m_cbPractical, &QCheckBox::toggled,
                this, [this](bool v){ m_chart->setPracticalVisible(v); });
        connect(m_cbStability, &QCheckBox::toggled,
                this, [this](bool v){ m_chart->setStabilityVisible(v); });
        connect(m_cbStClair,   &QCheckBox::toggled,
                this, [this](bool v){ m_chart->setStClairVisible(v);   });
        connect(m_cbSIL,       &QCheckBox::toggled,
                this, [this](bool v){ m_chart->setSILVisible(v);        });
        connect(m_cbThermal,   &QCheckBox::toggled,
                this, [this](bool v){ m_chart->setThermalVisible(v);    });
    }

    // ── Line cards scroll area ─────────────────────────────────────────────────
    m_cardsScroll = new QScrollArea();
    m_cardsScroll->setWidgetResizable(true);
    m_cardsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_cardsWidget = new QWidget();
    m_cardsLayout = new QVBoxLayout(m_cardsWidget);
    m_cardsLayout->setSpacing(4);
    m_cardsLayout->addStretch();

    m_cardsScroll->setWidget(m_cardsWidget);
    vl->addWidget(m_cardsScroll, 1);

    // ── Add line button ───────────────────────────────────────────────────────
    m_addBtn = new QPushButton("＋  Add Line");
    m_addBtn->setStyleSheet(
        "QPushButton{background:#27ae60;color:white;font-weight:bold;"
        "border-radius:4px;padding:4px;}"
        "QPushButton:hover{background:#2ecc71;}"
        "QPushButton:disabled{background:#aaa;}");
    connect(m_addBtn, &QPushButton::clicked, this, &LoadabilityTab::onAddLine);
    vl->addWidget(m_addBtn);

    // ── Calculate ─────────────────────────────────────────────────────────────
    m_calcBtn = new QPushButton("⚡  Calculate  [F5]");
    m_calcBtn->setFixedHeight(34);
    m_calcBtn->setStyleSheet(
        "QPushButton{font-weight:bold;font-size:12px;"
        "background:#2255aa;color:white;border-radius:4px;}"
        "QPushButton:hover{background:#3366cc;}"
        "QPushButton:pressed{background:#1a4488;}");
    connect(m_calcBtn, &QPushButton::clicked, this, &LoadabilityTab::onCalculate);
    vl->addWidget(m_calcBtn);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildRightPane
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::buildRightPane(QWidget* p)
{
    auto* vl = new QVBoxLayout(p);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(4);

    // Chart
    m_chart = new LoadabilityChartWidget();
    vl->addWidget(m_chart, 3);

    // Summary table
    auto* gb = new QGroupBox("Summary");
    auto* tl = new QVBoxLayout(gb);
    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels({
        "Line", "Zc (Ω)", "SIL (MVA)", "λ/4 (km)", "P thermal (MW)", "β (rad/km)"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setDefaultSectionSize(22);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    m_table->setMaximumHeight(160);
    tl->addWidget(m_table);
    vl->addWidget(gb, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// makeLineCard
// ─────────────────────────────────────────────────────────────────────────────
LineCard* LoadabilityTab::makeLineCard(int index)
{
    auto* card = new LineCard();

    card->group = new QGroupBox(QString("Line %1").arg(index + 1));
    card->group->setCheckable(false);

    auto* vl = new QVBoxLayout(card->group);
    vl->setSpacing(3);

    // ── Header row: label + import toggle + remove button ─────────────────────
    {
        auto* hl = new QHBoxLayout();
        card->lblEdit = new QLineEdit(QString("Line %1").arg(index + 1));
        card->lblEdit->setPlaceholderText("Label…");
        card->lblEdit->setMaximumWidth(100);

        card->importCb = new QCheckBox("Import from Parameters tab");
        card->importCb->setEnabled(m_lineTab != nullptr);

        card->removeBtn = new QPushButton("✕");
        card->removeBtn->setFixedSize(22,22);
        card->removeBtn->setToolTip("Remove this line");
        card->removeBtn->setStyleSheet(
            "QPushButton{background:#c0392b;color:white;border-radius:3px;font-weight:bold;}"
            "QPushButton:hover{background:#e74c3c;}");

        hl->addWidget(card->lblEdit);
        hl->addWidget(card->importCb);
        hl->addStretch();
        hl->addWidget(card->removeBtn);
        vl->addLayout(hl);
    }

    // ── Parameter form ────────────────────────────────────────────────────────
    auto* fl = new QFormLayout();
    fl->setLabelAlignment(Qt::AlignRight);
    fl->setSpacing(3);

    card->L_sb     = dSpin(0.001,  5.0,  1.0,   0.01, 4);
    card->C_sb     = dSpin(0.001,  1.0,  0.01,  0.001,5);
    card->r_sb     = dSpin(0.0,    2.0,  0.05,  0.01, 4);
    card->Vs_sb    = dSpin(1.0, 1200.0, 220.0,  10.0, 1);
    card->Vr_sb    = dSpin(1.0, 1200.0, 220.0,  10.0, 1);
    card->Vrate_sb = dSpin(1.0, 1200.0, 220.0,  10.0, 1);
    card->Ith_sb   = dSpin(1.0, 5000.0, 500.0,  50.0, 1);
    card->delta_sb = dSpin(5.0,   89.0,  30.0,   1.0, 1);
    card->freq_sb  = dSpin(1.0,  200.0,  50.0,   1.0, 1);
    card->maxLen_sb= dSpin(50.0,1500.0, 600.0,  50.0, 0);

    fl->addRow("L (mH/km):",      card->L_sb);
    fl->addRow("C (µF/km):",      card->C_sb);
    fl->addRow("R (Ω/km):",       card->r_sb);
    fl->addRow("Vs (kV):",        card->Vs_sb);
    fl->addRow("Vr (kV):",        card->Vr_sb);
    fl->addRow("V rated (kV):",   card->Vrate_sb);
    fl->addRow("I thermal (A):",  card->Ith_sb);
    fl->addRow("δ practical (°):",card->delta_sb);
    fl->addRow("Frequency (Hz):", card->freq_sb);
    fl->addRow("Max length (km):",card->maxLen_sb);

    vl->addLayout(fl);

    // ── Wire up import toggle ─────────────────────────────────────────────────
    int idx = index;
    connect(card->importCb, &QCheckBox::toggled, this, [this, idx](bool checked){
        onImportToggled(idx, checked);
    });

    // ── Wire up remove ────────────────────────────────────────────────────────
    connect(card->removeBtn, &QPushButton::clicked, this, [this, card](){
        int i = m_cards.indexOf(card);
        if (i >= 0) onRemoveLine(i);
    });

    return card;
}

// ─────────────────────────────────────────────────────────────────────────────
// addCardToLayout — insert before the stretch
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::addCardToLayout(LineCard* card)
{
    int stretchIdx = m_cardsLayout->count() - 1;
    m_cardsLayout->insertWidget(stretchIdx, card->group);
}

// ─────────────────────────────────────────────────────────────────────────────
// updateCardIndices — renumber group titles and labels after add/remove
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::updateCardIndices()
{
    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i]->group->setTitle(QString("Line %1").arg(i + 1));
        // Reconnect import toggle with correct index
        disconnect(m_cards[i]->importCb, nullptr, nullptr, nullptr);
        int idx = i;
        connect(m_cards[i]->importCb, &QCheckBox::toggled,
                this, [this, idx](bool checked){ onImportToggled(idx, checked); });
        // Reconnect remove button
        disconnect(m_cards[i]->removeBtn, nullptr, nullptr, nullptr);
        LineCard* card = m_cards[i];
        connect(m_cards[i]->removeBtn, &QPushButton::clicked, this, [this, card](){
            int j = m_cards.indexOf(card);
            if (j >= 0) onRemoveLine(j);
        });
    }
    m_addBtn->setEnabled(m_cards.size() < kMaxLines);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::onAddLine()
{
    if (m_cards.size() >= kMaxLines) return;
    LineCard* card = makeLineCard(m_cards.size());
    m_cards.append(card);
    addCardToLayout(card);
    updateCardIndices();
}

void LoadabilityTab::onRemoveLine(int index)
{
    if (m_cards.size() <= 1) return;   // always keep at least one
    LineCard* card = m_cards.takeAt(index);
    m_cardsLayout->removeWidget(card->group);
    card->group->deleteLater();
    delete card;
    updateCardIndices();
}

void LoadabilityTab::onImportToggled(int index, bool checked)
{
    if (index < 0 || index >= m_cards.size()) return;
    LineCard* card = m_cards[index];

    // Disable manual entry while import is active
    for (auto* w : {card->L_sb, card->C_sb, card->r_sb,
                    card->Vs_sb, card->Vr_sb, card->Vrate_sb,
                    card->Ith_sb, card->freq_sb})
        w->setEnabled(!checked);

    if (checked && m_lineTab)
        importFromLineTab(*card);
}

void LoadabilityTab::notifyLineParamsUpdated()
{
    // Auto-refresh any cards with import enabled
    for (int i = 0; i < m_cards.size(); ++i) {
        if (m_cards[i]->importCb->isChecked() && m_lineTab)
            importFromLineTab(*m_cards[i]);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// importFromLineTab
// Pulls the last-computed single-circuit results from LineParametersTab.
// LineParametersTab exposes a currentInput() / currentResults() getter.
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::importFromLineTab(LineCard& card) const
{
    if (!m_lineTab) return;
    const GeometryInput g = m_lineTab->currentInput();
    // Compute L, C from geometry so we always have fresh values
    try {
        LineResults r = TransmissionLine::compute(g);
        card.L_sb    ->setValue(r.inductance_mH_km);
        card.C_sb    ->setValue(r.capacitance_uF_km);
        card.r_sb    ->setValue(g.r_ac);
        card.Vs_sb   ->setValue(g.voltageKV);
        card.Vr_sb   ->setValue(g.voltageKV);
        card.Vrate_sb->setValue(g.voltageKV);
        card.Ith_sb  ->setValue(g.currentA);
        card.freq_sb ->setValue(g.freq);
    } catch (...) {}
}

// ─────────────────────────────────────────────────────────────────────────────
// collectCard
// ─────────────────────────────────────────────────────────────────────────────
LoadabilityInput LoadabilityTab::collectCard(const LineCard& card) const
{
    LoadabilityInput in;
    in.label     = card.lblEdit->text().trimmed();
    if (in.label.isEmpty()) in.label = card.group->title();
    in.L_mH_km   = card.L_sb    ->value();
    in.C_uF_km   = card.C_sb    ->value();
    in.r_ac      = card.r_sb    ->value();
    in.Vs_kV     = card.Vs_sb   ->value();
    in.Vr_kV     = card.Vr_sb   ->value();
    in.Vrated_kV = card.Vrate_sb->value();
    in.I_th_A    = card.Ith_sb  ->value();
    in.delta_deg = card.delta_sb->value();
    in.freq      = card.freq_sb ->value();
    in.maxLen_km = (int)card.maxLen_sb->value();
    return in;
}

// ─────────────────────────────────────────────────────────────────────────────
// onCalculate
// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityTab::onCalculate()
{
    QVector<LoadabilityResult> results;

    for (const auto* card : m_cards) {
        // Refresh import if active
        if (card->importCb->isChecked() && m_lineTab)
            importFromLineTab(*const_cast<LineCard*>(card));

        try {
            results.append(LoadabilityCalc::compute(collectCard(*card)));
        } catch (const std::exception& e) {
            Q_UNUSED(e);
        }
    }

    m_chart->setResults(results);

    // Pass labels so the legend shows user-defined line names
    QStringList labels;
    for (const auto* card : m_cards)
        labels << (card->lblEdit->text().trimmed().isEmpty()
                       ? card->group->title()
                       : card->lblEdit->text().trimmed());
    m_chart->setLabels(labels);

    m_chart->setPracticalVisible(m_cbPractical->isChecked());
    m_chart->setStabilityVisible(m_cbStability->isChecked());
    m_chart->setStClairVisible  (m_cbStClair  ->isChecked());
    m_chart->setSILVisible      (m_cbSIL      ->isChecked());
    m_chart->setThermalVisible  (m_cbThermal  ->isChecked());

    // ── Summary table ─────────────────────────────────────────────────────────
    m_table->setRowCount(results.size());
    for (int i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        QString lbl   = m_cards[i]->lblEdit->text().trimmed();
        if (lbl.isEmpty()) lbl = QString("Line %1").arg(i+1);

        auto cell = [&](int col, const QString& txt){
            auto* item = new QTableWidgetItem(txt);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(i, col, item);
        };
        m_table->setItem(i, 0, new QTableWidgetItem(lbl));
        cell(1, QString::number(r.Zc_ohm,      'f', 2));
        cell(2, QString::number(r.SIL_MVA,     'f', 1));
        cell(3, QString::number(r.quarter_wave_km,'f',1));
        cell(4, QString::number(r.P_thermal_MW,'f', 1));
        cell(5, QString::number(r.beta_rad_km, 'f', 6));
    }
}

} // namespace LineTool
