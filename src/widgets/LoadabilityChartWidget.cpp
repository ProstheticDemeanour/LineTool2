// ─────────────────────────────────────────────────────────────────────────────
// widgets/LoadabilityChartWidget.cpp
//
// Pure QPainter chart — no QtCharts, no white background problems.
// Colours derived entirely from palette() so it blends with the app surface.
// ─────────────────────────────────────────────────────────────────────────────
#include "LoadabilityChartWidget.h"

#include <QPainter>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <cmath>
#include <algorithm>

namespace LineTool {

// ── Per-line colour palette — muted tones that work on a mid-grey surface ────
static const QColor kPalette[] = {
    QColor( 90, 160, 220),   // steel blue
    QColor(210,  90,  80),   // muted red
    QColor( 80, 180, 120),   // sage green
    QColor(180, 120, 200),   // dusty purple
    QColor(220, 150,  60),   // amber
    QColor( 70, 190, 180),   // teal
    QColor(200, 190,  80),   // khaki
    QColor(160, 160, 160),   // neutral grey
};
static const int kPaletteSize = 8;

// ─────────────────────────────────────────────────────────────────────────────
LoadabilityChartWidget::LoadabilityChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(300, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // No setAutoFillBackground — inherit parent colour naturally, just like
    // SheathGraphWidget. paintEvent fills the rect explicitly.
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityChartWidget::setResults(const QVector<LoadabilityResult>& r)
{
    m_results = r;
    update();
}

void LoadabilityChartWidget::setThermalVisible  (bool v){ m_showThermal  =v; update(); }
void LoadabilityChartWidget::setStabilityVisible(bool v){ m_showStability=v; update(); }
void LoadabilityChartWidget::setPracticalVisible(bool v){ m_showPractical=v; update(); }
void LoadabilityChartWidget::setStClairVisible  (bool v){ m_showStClair  =v; update(); }
void LoadabilityChartWidget::setSILVisible      (bool v){ m_showSIL      =v; update(); }

// ─────────────────────────────────────────────────────────────────────────────
double LoadabilityChartWidget::niceStep(double range, int ticks)
{
    double raw  = range / ticks;
    double mag  = std::pow(10.0, std::floor(std::log10(raw)));
    double norm = raw / mag;
    double nice = (norm < 1.5) ? 1.0 : (norm < 3.5) ? 2.0 :
                  (norm < 7.5) ? 5.0 : 10.0;
    return nice * mag;
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityChartWidget::collectVisible(QVector<VisibleSeries>& out,
                                             const LoadabilityResult& r,
                                             int pi) const
{
    const QColor col = kPalette[pi % kPaletteSize];

    if (m_showPractical && !r.practical.x.isEmpty())
        out.push_back({ &r.practical, col,              2.5f, Qt::SolidLine    });
    if (m_showStability && !r.stability.x.isEmpty())
        out.push_back({ &r.stability, col.lighter(170), 1.5f, Qt::DashLine     });
    if (m_showStClair   && !r.stclair.x.isEmpty())
        out.push_back({ &r.stclair,   col.lighter(140), 2.0f, Qt::DashDotLine  });
    if (m_showThermal   && !r.thermal.x.isEmpty())
        out.push_back({ &r.thermal,   col.darker(120),  1.5f, Qt::DotLine      });
    if (m_showSIL       && !r.sil.x.isEmpty()) {
        QColor sc = col; sc.setAlphaF(0.5f);
        out.push_back({ &r.sil,       sc,               1.0f, Qt::SolidLine    });
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadabilityChartWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // ── Colours from palette — always matches app theme ───────────────────────
    const QColor bgCol   = palette().color(QPalette::Window);
    const QColor textCol = palette().color(QPalette::WindowText);
    const QColor midCol  = palette().color(QPalette::Mid);
    // Plot area: just a 4% darker step — enough to define the boundary
    // without creating a visible grey box on light themes
    const QColor plotBg = bgCol.darker(104);
    const QColor gridCol = midCol;

    // ── Fill entire widget with window background ─────────────────────────────
    p.fillRect(rect(), bgCol);

    // ── Margins ───────────────────────────────────────────────────────────────
    const int W    = width();
    const int H    = height();
    const int padL = 70;
    const int padR = 20;
    const int padT = 36;
    const int padB = 56;   // extra room for legend

    const int plotW = W - padL - padR;
    const int plotH = H - padT - padB;
    if (plotW < 40 || plotH < 40) return;

    // ── Empty state ───────────────────────────────────────────────────────────
    if (m_results.isEmpty()) {
        p.setPen(midCol);
        QFont f = p.font(); f.setPointSize(11); p.setFont(f);
        p.drawText(QRect(padL, padT, plotW, plotH),
                   Qt::AlignCenter, "Press Calculate to populate");
        return;
    }

    // ── Collect all visible series and find data ranges ───────────────────────
    QVector<VisibleSeries> series;
    double xMax = 0, yMax = 0;

    for (int i = 0; i < m_results.size(); ++i) {
        collectVisible(series, m_results[i], i);
        const auto& r = m_results[i];
        for (auto* cs : { &r.practical, &r.stability, &r.stclair, &r.thermal, &r.sil }) {
            if (!cs->x.isEmpty()) xMax = std::max(xMax, cs->x.last());
            for (double y : cs->y) yMax = std::max(yMax, y);
        }
    }
    if (xMax <= 0 || yMax <= 0) return;

    // ── Nice axis scales ──────────────────────────────────────────────────────
    const int    nTicksY  = 6;
    const int    nTicksX  = 7;
    const double yStep    = niceStep(yMax * 1.05, nTicksY);
    const double yTop     = yStep * std::ceil(yMax * 1.05 / yStep);
    const double xStep    = niceStep(xMax * 1.05, nTicksX);
    const double xRight   = xStep * std::ceil(xMax * 1.05 / xStep);

    auto toX = [&](double v) -> double {
        return padL + v / xRight * plotW;
    };
    auto toY = [&](double v) -> double {
        return padT + (1.0 - v / yTop) * plotH;
    };

    // ── Plot area background ──────────────────────────────────────────────────
    p.fillRect(padL, padT, plotW, plotH, plotBg);

    // ── Grid lines ────────────────────────────────────────────────────────────
    QFont smallFont = p.font();
    smallFont.setPointSize(8);
    p.setFont(smallFont);
    QFontMetrics fm(smallFont);

    // Y grid
    p.setPen(QPen(gridCol, 1, Qt::DotLine));
    for (int t = 0; t <= nTicksY * 2; ++t) {
        double v = yStep * 0.5 * t;
        if (v > yTop * 1.001) break;
        double y = toY(v);
        p.drawLine(QPointF(padL, y), QPointF(padL + plotW, y));

        if (t % 2 == 0) {
            QString lbl = (v >= 1000.0)
                ? QString("%1k").arg(v / 1000.0, 0, 'f', 1)
                : QString::number((int)std::round(v));
            p.setPen(textCol);
            p.drawText(0, (int)y - fm.height()/2, padL - 6, fm.height(),
                       Qt::AlignRight | Qt::AlignVCenter, lbl);
            p.setPen(QPen(gridCol, 1, Qt::DotLine));
        }
    }

    // X grid
    for (int t = 0; t <= nTicksX * 2; ++t) {
        double v = xStep * 0.5 * t;
        if (v > xRight * 1.001) break;
        double x = toX(v);
        p.drawLine(QPointF(x, padT), QPointF(x, padT + plotH));

        if (t % 2 == 0) {
            QString lbl = (v >= 1000.0)
                ? QString("%1k").arg(v / 1000.0, 0, 'f', 0)
                : QString::number((int)std::round(v));
            p.setPen(textCol);
            int lw = fm.horizontalAdvance(lbl);
            p.drawText((int)x - lw/2, padT + plotH + 4,
                       lw + 4, fm.height() + 2, Qt::AlignHCenter, lbl);
            p.setPen(QPen(gridCol, 1, Qt::DotLine));
        }
    }

    // ── Plot border ───────────────────────────────────────────────────────────
    p.setPen(QPen(midCol, 1));
    p.drawRect(padL, padT, plotW, plotH);

    // ── Curves ───────────────────────────────────────────────────────────────
    p.setClipRect(padL, padT, plotW, plotH);
    for (const auto& s : series) {
        QPen pen(s.color, s.width, s.style);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        p.setPen(pen);

        const int n = s.data->x.size();
        for (int i = 1; i < n; ++i) {
            p.drawLine(QPointF(toX(s.data->x[i-1]), toY(s.data->y[i-1])),
                       QPointF(toX(s.data->x[i]),   toY(s.data->y[i])));
        }
    }
    p.setClipping(false);

    // ── Axis titles ───────────────────────────────────────────────────────────
    p.setPen(textCol);
    QFont axFont = p.font(); axFont.setPointSize(8); p.setFont(axFont);

    // X title
    p.drawText(padL, padT + plotH + fm.height() + 6, plotW, fm.height() + 4,
               Qt::AlignHCenter, "Line Length  (km)");

    // Y title — rotated
    p.save();
    p.translate(12, padT + plotH / 2);
    p.rotate(-90);
    p.drawText(-40, -fm.height(), 80, fm.height() * 2, Qt::AlignHCenter, "Power  (MW)");
    p.restore();

    // ── Title ─────────────────────────────────────────────────────────────────
    QFont titleFont = p.font();
    titleFont.setPointSize(9);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    p.setFont(titleFont);
    p.setPen(textCol);
    QString title = QString("TRANSMISSION LINE LOADABILITY   (%1 line%2)")
                        .arg(m_results.size())
                        .arg(m_results.size() > 1 ? "s" : "");
    p.drawText(padL, 4, plotW, padT - 4, Qt::AlignHCenter | Qt::AlignVCenter, title);
    p.setFont(smallFont);

    // ── Legend ────────────────────────────────────────────────────────────────
    // One row per line, columns per curve type
    const struct { bool* show; const char* label; Qt::PenStyle style; float w; } kTypes[] = {
        { &m_showPractical, "Practical",  Qt::SolidLine,   2.5f },
        { &m_showStability, "Stability",  Qt::DashLine,    1.5f },
        { &m_showStClair,   "St. Clair",  Qt::DashDotLine, 2.0f },
        { &m_showThermal,   "Thermal",    Qt::DotLine,     1.5f },
        { &m_showSIL,       "SIL",        Qt::SolidLine,   1.0f },
    };

    int legX = padL;
    int legY = H - 18;
    int swatchW = 22;
    int colW    = 90;

    for (int i = 0; i < m_results.size(); ++i) {
        QColor col = kPalette[i % kPaletteSize];
        QString lineLabel = (m_cards_labels.size() > i)
                                ? m_cards_labels[i]
                                : QString("Line %1").arg(i+1);

        // Line label
        p.setPen(col);
        QFont lf = smallFont; lf.setBold(true); p.setFont(lf);
        p.drawText(legX, legY - fm.height() - 2, 60, fm.height(),
                   Qt::AlignLeft, lineLabel);
        p.setFont(smallFont);

        int cx = legX + 62;
        for (int t = 0; t < 5; ++t) {
            if (!(*kTypes[t].show)) continue;

            // Colour modifier matching collectVisible
            QColor c = col;
            if      (t == 1) c = col.lighter(170);
            else if (t == 2) c = col.lighter(140);
            else if (t == 3) c = col.darker(120);
            else if (t == 4) { c = col; c.setAlphaF(0.5f); }

            QPen lp(c, kTypes[t].w, kTypes[t].style);
            p.setPen(lp);
            p.drawLine(cx, legY - fm.height()/2, cx + swatchW, legY - fm.height()/2);
            p.setPen(textCol);
            p.drawText(cx + swatchW + 2, legY - fm.height(),
                       colW - swatchW - 4, fm.height() + 2,
                       Qt::AlignLeft, kTypes[t].label);
            cx += colW;
        }
        legY -= fm.height() + 4;
    }
}

} // namespace LineTool
